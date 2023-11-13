/*
 * Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

section AmazonTrinoAADConnector;

// When set to true, additional trace information will be written out to the User log. 
// This should be set to false before release. Tracing is done through a call to 
// Diagnostics.LogValue(). When EnableTraceOutput is set to false, the call becomes a 
// no-op and simply returns the original value.
EnableTraceOutput = false;

[DataSource.Kind="AmazonTrinoAADConnector", Publish="AmazonTrinoAADConnector.Publish"]
shared AmazonTrinoAADConnector.Contents = Value.ReplaceType(AmazonTrinoAADConnectorImpl, AmazonTrinoAADConnectorType);

// Wrapper function to provide additional UI customization.
AmazonTrinoAADConnectorType = type function (
        Region as (type text meta [
            Documentation.FieldCaption = "Region",
            Documentation.FieldDescription = "The AWS Region.",
            Documentation.SampleValues = { "us-east-1" }
        ]),
        optional AADApplicationID as (type text meta [
            Documentation.FieldCaption = "AAD Application ID",
            Documentation.FieldDescription = "The unique id of the registered application on Azure AD.",
            DataSource.Path = true
        ]),
        optional AADTenant as (type text meta [
            Documentation.FieldCaption = "AAD Tenant",
            Documentation.FieldDescription = "The Azure AD Tenant ID.",
            DataSource.Path = true
        ]),
        optional AADClientSecret as (type text meta [
            Documentation.FieldCaption = "AAD Client Secret",
            Documentation.FieldDescription = "The unique id of the registered application on Azure AD.",
            DataSource.Path = true
        ]),
        optional RoleArn as (type text meta [
            Documentation.FieldCaption = "Role Arn",
            Documentation.FieldDescription = "The Amazon Resource Name (ARN) of the role that the caller is assuming.",
            DataSource.Path = true
        ]),
        optional IdpArn as (type text meta [
            Documentation.FieldCaption = "IdP Arn",
            Documentation.FieldDescription = "The Amazon Resource Name (ARN) of the SAML provider in IAM that describes the Idp.",
            DataSource.Path = true
        ]),
        optional ProfileName as (type text meta [
            Documentation.FieldCaption = "Profile Name",
            Documentation.FieldDescription = "The AWS profile name for Amazon Trino.",
            Documentation.SampleValues = { "default" },
            DataSource.Path = true
        ])
    )
    as table meta [
        Documentation.Name = "Amazon Trino"
    ];

AmazonTrinoAADConnectorImpl = (
    Region as text,
    optional AADApplicationID  as text,
    optional AADTenant as text,
    optional AADClientSecret as text,
    optional RoleArn as text,
    optional IdpArn as text,
    optional ProfileName as text
    ) as table =>
    let
        Credential = Extension.CurrentCredential(),
        AuthenticationMode = Credential[AuthenticationKind],

        // Sets connection string properties for encrypted connections.
        EncryptedConnectionString =
            if Credential[EncryptConnection] = null or Credential[EncryptConnection] = true then
                [
                    SSL = 1
                ]
            else
                [
                    SSL = 0
                ],

        AuthenticationString = 
            if AuthenticationMode = "UsernamePassword" then
                [
                    UID = Credential[Username],
                    PWD = Credential[Password],
                    Auth = "AAD",
                    IdpName = "AzureAD"
                ]
            else [],

        AADApplicationIDParameter = if AADApplicationID  <> null then [ AADApplicationID  = AADApplicationID  ] else [],
        AADTenantParameter = if AADTenant  <> null then [ AADTenant  = AADTenant  ] else [],
        AADClientSecretParameter = if AADClientSecret  <> null then [ AADClientSecret  = AADClientSecret  ] else [],
        RoleArnParameter = if RoleArn  <> null then [ RoleArn  = RoleArn  ] else [],
        IdpArnParameter = if IdpArn  <> null then [ IdpARN  = IdpArn  ] else [],
        ProfileNameParameter = if ProfileName <> null then [ ProfileName = ProfileName ]  else [],

        BaseConnectionString = [
            Driver = "Amazon Trino ODBC Driver",
            Region = Region
        ],

        ConnectionString = BaseConnectionString & AuthenticationString & AADApplicationIDParameter & AADTenantParameter & AADClientSecretParameter & RoleArnParameter & IdpArnParameter & ProfileNameParameter,

        SqlCapabilities = Diagnostics.LogValue("SqlCapabilities_Options", [
            SupportsTop = false,
            LimitClauseKind = LimitClauseKind.LimitOffset,
            Sql92Conformance = ODBC[SQL_SC][SQL_SC_SQL92_FULL],
            SupportsNumericLiterals = true,
            SupportsStringLiterals = true,
            SupportsOdbcDateLiterals = true,
            SupportsOdbcTimeLiterals = true,
            SupportsOdbcTimestampLiterals = true
        ]),

        // SQLColumns is a function handler that receives the results of an ODBC call to SQLColumns().
        SQLColumns = (catalogName, schemaName, tableName, columnName, source) =>
            if (EnableTraceOutput <> true) then source else
            // the if statement conditions will force the values to evaluated/written to diagnostics
            if (Diagnostics.LogValue("SQLColumns.TableName", tableName) <> "***" and Diagnostics.LogValue("SQLColumns.ColumnName", columnName) <> "***") then
                let
                    // Outputting the entire table might be too large, and result in the value being truncated.
                    // We can output a row at a time instead with Table.TransformRows()
                    rows = Table.TransformRows(source, each Diagnostics.LogValue("SQLColumns", _)),
                    toTable = Table.FromRecords(rows)
                in
                    Value.ReplaceType(toTable, Value.Type(source))
            else
                source,

        SQLGetInfo = Diagnostics.LogValue("SQLGetInfo_Options", [
            SQL_AGGREGATE_FUNCTIONS = ODBC[SQL_AF][All],
            SQL_SQL_CONFORMANCE = ODBC[SQL_SC][SQL_SC_SQL92_INTERMEDIATE]
        ]),

        SQLGetFunctions = Diagnostics.LogValue("SQLGetFunctions_Options", [
            SQL_API_SQLBINDPARAMETER = false
        ]),

        SQLGetTypeInfo = (types) => 
            if (EnableTraceOutput <> true) then types else
            let
                // Outputting the entire table might be too large, and result in the value being truncated.
                // We can output a row at a time instead with Table.TransformRows()
                rows = Table.TransformRows(types, each Diagnostics.LogValue("SQLGetTypeInfo " & _[TYPE_NAME], _)),
                toTable = Table.FromRecords(rows)
            in
                Value.ReplaceType(toTable, Value.Type(types)),

        AstVisitor = [
            Constant =
                let
                    Quote = each Text.Format("'#{0}'", { _ }),
                    Cast = (value, typeName) => [
                        Text = Text.Format("CAST(#{0} as #{1})", { value, typeName })
                    ],
                    Visitor = [
                        timestamp = each Cast(Quote(DateTime.ToText(_, "yyyy-MM-dd HH:mm:ss.fffffff")), "timestamp")
                    ]
                in
                    (typeInfo, ast) => Record.FieldOrDefault(Visitor, typeInfo[TYPE_NAME], each null)(ast[Value])
        ],
        
        ConnectionOptions = [
            // View the tables grouped by their schema names.
            HierarchicalNavigation = true,
            
            // Handlers for ODBC driver capabilities.
            SqlCapabilities = SqlCapabilities,
            SQLColumns = SQLColumns,
            SQLGetInfo = SQLGetInfo,
            SQLGetFunctions = SQLGetFunctions,
            SQLGetTypeInfo = SQLGetTypeInfo,

            // For modifying generated SQL.
            AstVisitor = AstVisitor,
            
            // Handles ODBC Errors.
            OnError = OnOdbcError,
            
            // Connection string properties used for encrypted connections.
            CredentialConnectionString = EncryptedConnectionString
        ],

        OdbcDatasource = Odbc.DataSource(ConnectionString, ConnectionOptions)
    in
        OdbcDatasource;

// Handles ODBC errors.
OnOdbcError = (errorRecord as record) =>
    let
        ErrorMessage = errorRecord[Message],

        IsDriverNotInstalled = Text.Contains(ErrorMessage, "doesn't correspond to an installed ODBC driver"),

        OdbcError = errorRecord[Detail][OdbcErrors]{0},
        OdbcErrorCode = OdbcError[NativeError],
        
        // Failed to connect to given host.
        IsHostUnreachable = (OdbcErrorCode = 202)
    in
        if IsDriverNotInstalled then
            error Error.Record(errorRecord[Reason], "The Amazon Trino ODBC driver is not installed. Please install the driver.", errorRecord[Detail])
        else if IsHostUnreachable then
            error Error.Record(errorRecord[Reason], "Could not reach server. Please double-check the authentication.", errorRecord[Detail])
        else 
            error errorRecord;

// Data Source Kind description.
AmazonTrinoAADConnector = [
    Label = Extension.LoadString(""),

    SupportsEncryption = true,
    Authentication = [
        Implicit = [
            Label = "AWS Profile"
        ],
        UsernamePassword = [
            Label = "Azure Active Directory"
        ]
    ],
    
    // Needed for use with Power BI Service.
    TestConnection = (dataSourcePath) => 
        let
            json = Json.Document(dataSourcePath),
            Region = json[Region],
            AADApplicationID  = json[AADApplicationID],
            AADTenant = json[AADTenant],
            AADClientSecret = json[AADClientSecret],
            RoleArn = json[RoleArn],
            IdpArn = json[IdpARN],
            ProfileName = json[ProfileName]
        in
            { "AmazonTrinoAADConnector.Contents", Region, AADApplicationID , AADTenant, AADClientSecret, RoleArn, IdpArn, ProfileName }
];

// Data Source UI publishing description.
AmazonTrinoAADConnector.Publish = [
    Category = "Other",
    SupportsDirectQuery = true,

    ButtonText = { Extension.LoadString("ButtonTitle"), Extension.LoadString("ButtonHelp") },
    LearnMoreUrl = "https://aws.amazon.com/AmazonTrino/",

    SourceImage = AmazonTrinoAADConnector.Icons,
    SourceTypeImage = AmazonTrinoAADConnector.Icons
];

AmazonTrinoAADConnector.Icons = [
    Icon16 = { Extension.Contents("AmazonTrino16.png"), Extension.Contents("AmazonTrino20.png"), Extension.Contents("AmazonTrino24.png"), Extension.Contents("AmazonTrino32.png") },
    Icon32 = { Extension.Contents("AmazonTrino32.png"), Extension.Contents("AmazonTrino40.png"), Extension.Contents("AmazonTrino48.png"), Extension.Contents("AmazonTrino64.png") }
];

// Loads functions from another project file.
Extension.LoadFunction = (name as text) =>
    let
        binary = Extension.Contents(name),
        asText = Text.FromBinary(binary)
    in
        Expression.Evaluate(asText, #shared);

// Diagnostics module contains multiple functions.
Diagnostics = Extension.LoadFunction("Diagnostics.pqm");
Diagnostics.LogValue = if (EnableTraceOutput) then Diagnostics[LogValue] else (prefix, value) => value;

// OdbcConstants contains numeric constants from the ODBC header files, and helper function to create bitfield values.
ODBC = Extension.LoadFunction("OdbcConstants.pqm");
