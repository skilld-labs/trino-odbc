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

section AmazonTrinoConnectorUnitTests;

shared AmazonTrinoConnector.UnitTest =
[
    Region = "us-west-2",
    ProfileName = "default", // Enter profile here
    Token = "", // Enter session token here
    Source = AmazonTrinoConnector.Contents(Region, Token, ProfileName),
    PowerBI_DB = Source{[Name="PowerBI",Kind="Database"]}[Data],
    DevOpsOriginal = PowerBI_DB{[Name="DevOps", Kind="Table"]}[Data],
    
    // Renaming column because the ":" character can be problematic.
    DevOps = Table.RenameColumns(DevOpsOriginal, {"measure_value::double", "measure_value"}),

    HostNamePivot = "HOST-QLUVR",
    UpperCaseHostNames = (hostname) => Text.Upper(hostname),
    LowerCaseSomeHostNames = (hostname) => if hostname < HostNamePivot then hostname else Text.Lower(hostname),
    
    // LeftTable has an all upper case hostname column 
    // while RightTable has upper case hostname for every hostname before the pivot 
    // and lower case hostname for every hostname after the pivot.
    // As a result the tables' hostname is overlapping before the pivot and non-overlapping after the pivot.
    LeftTable = Table.TransformColumns(DevOps, {"hostname", UpperCaseHostNames}),
    RightTable = Table.TransformColumns(LeftTable, {"hostname", LowerCaseSomeHostNames}),

    AllRowsSatisfyCondition = (table, condition) =>
        let
            Filter = Table.SelectRows(table, condition)
        in
            Table.RowCount(Filter) = Table.RowCount(table),

    NRowsSatisfyCondition = (table, condition, n) => 
        let
            Filter = Table.SelectRows(table, condition)
        in
            Table.RowCount(Filter) = n,

    facts =
    {
        // Basic
        Fact("Source has 4 columns",
             4,
             Table.ColumnCount(Source)
        ),
        Fact("DevOps table has data",
             true,
             not Table.IsEmpty(DevOps)
        ),
        Fact("DevOps table has 6 columns",
             6,
             Table.ColumnCount(DevOps)
        ),
        Fact("DevOps table has the right fields",
            { "hostname", "az", "region", "measure_name", "time", "measure_value" },
            Record.FieldNames(Type.RecordFields(Type.TableRow(Value.Type(DevOps))))
        ),

        // Filters
        Fact("Operators",
             true,
             let
                Filtered = Table.SelectRows(DevOps, each [measure_name] = "cpu_utilization")
             in
                Table.RowCount(Filtered) < Table.RowCount(DevOps)
        ),
        Fact("And operation",
             true,
             let
                Filter1 = Table.SelectRows(DevOps, each [measure_name] = "cpu_utilization"),
                AndFilter = Table.SelectRows(DevOps, each [measure_name] = "cpu_utilization" and [measure_value] > 40)
             in
                Table.RowCount(AndFilter) < Table.RowCount(Filter1)
        ),
        Fact("Or operation",
             true,
             let
                Filter1 = Table.SelectRows(DevOps, each [measure_name] = "cpu_utilization"),
                Filter2 = Table.SelectRows(DevOps, each [measure_value] > 40),
                OrFilter = Table.SelectRows(DevOps, each [measure_name] = "cpu_utilization" or [measure_value] > 40)
             in
                Table.RowCount(OrFilter) > Table.RowCount(Filter1) and Table.RowCount(OrFilter) > Table.RowCount(Filter2)
        ),
        Fact("LIKE operation",
             true,
             let
                Filtered = Table.SelectRows(DevOps, each Text.StartsWith([az], "ap-"))
             in
                Table.RowCount(Filtered) > 0 and Table.RowCount(Filtered) < Table.RowCount(DevOps)
        ),

        // Joins
        Fact("Left Outer Join",
             true,
             let
                ValidRowLeftJoin = (row) =>
                    if row[hostname] < HostNamePivot then
                        AllRowsSatisfyCondition(row[Join], each [hostname] = row[hostname])
                    else Table.IsEmpty(row[Join]),
                ValidLeftJoin = (table) =>
                    NRowsSatisfyCondition(table, each [hostname] is null, 0) and
                    AllRowsSatisfyCondition(table, each ValidRowLeftJoin(_)),
                Join = Table.NestedJoin(LeftTable, {"hostname"}, RightTable, {"hostname"}, "Join", JoinKind.LeftOuter)
             in
                ValidLeftJoin(Join)
        ),
        Fact("Right Outer Join",
             true,
             let
                 ValidRowRightJoin = (row) =>
                    if row[hostname] is null then
                        AllRowsSatisfyCondition(row[Join], each [hostname] >= HostNamePivot)
                    else if row[hostname] < HostNamePivot then
                        AllRowsSatisfyCondition(row[Join], each [hostname] = row[hostname])
                    else false,
                ValidRightJoin = (table) =>
                    NRowsSatisfyCondition(table, each [hostname] is null, 1) and
                    AllRowsSatisfyCondition(table, each ValidRowRightJoin(_)),
                Join = Table.NestedJoin(LeftTable, {"hostname"}, RightTable, {"hostname"}, "Join", JoinKind.RightOuter)
             in
                ValidRightJoin(Join)
        ),
        Fact("Outer Join",
             true,
             let
                ValidRowOuterJoin = (row) =>
                    if row[hostname] is null then
                        AllRowsSatisfyCondition(row[Join], each [hostname] >= HostNamePivot)
                    else if row[hostname] < HostNamePivot then
                        AllRowsSatisfyCondition(row[Join], each [hostname] = row[hostname])
                    else Table.RowCount(row[Join]) = 1 and row[Join]{0}[hostname] is null,
                ValidOuterJoin = (table) =>
                    NRowsSatisfyCondition(table, each [hostname] is null, 1) and
                    AllRowsSatisfyCondition(table, each ValidRowOuterJoin(_)),
                Join = Table.NestedJoin(LeftTable, {"hostname"}, RightTable, {"hostname"}, "Join", JoinKind.FullOuter)
             in
                ValidOuterJoin(Join)
        ),
        Fact("Inner Join",
             true,
             let
                ValidRowInnerJoin = (row) =>
                    if row[hostname] < HostNamePivot then
                        AllRowsSatisfyCondition(row[Join], each [hostname] = row[hostname])
                    else false,
                ValidInnerJoin = (table) =>
                    NRowsSatisfyCondition(table, each [hostname] is null, 0) and
                    AllRowsSatisfyCondition(table, each ValidRowInnerJoin(_)),
                Join = Table.NestedJoin(LeftTable, {"hostname"}, RightTable, {"hostname"}, "Join", JoinKind.Inner)
             in
                ValidInnerJoin(Join)
        ),
        Fact("Left Anti Join",
             true,
             let
                ValidRowLeftAntiJoin = (row) => 
                    if row[hostname] >= HostNamePivot then 
                        AllRowsSatisfyCondition(row[Join], each [hostname] is null) 
                    else false,
                ValidLeftAntiJoin = (table) =>
                    NRowsSatisfyCondition(table, each [hostname] is null, 0) and
                    AllRowsSatisfyCondition(table, each ValidRowLeftAntiJoin(_)),
                Join = Table.NestedJoin(LeftTable, {"hostname"}, RightTable, {"hostname"}, "Join", JoinKind.LeftAnti)
             in
                ValidLeftAntiJoin(Join)
        ),
        Fact("Right Anti Join",
             true,
             let
                ValidRowRightAntiJoin = (row) =>
                    if row[hostname] is null then
                        AllRowsSatisfyCondition(row[Join], each [hostname] >= HostNamePivot)
                    else false,
                ValidRightAntiJoin = (table) =>
                    Table.RowCount(table) = 1 and table{0}[hostname] is null and
                    AllRowsSatisfyCondition(table, each ValidRowRightAntiJoin(_)),
                Join = Table.NestedJoin(LeftTable, {"hostname"}, RightTable, {"hostname"}, "Join", JoinKind.RightAnti)
             in
                ValidRightAntiJoin(Join)
        ),

        // Aggregates
        Fact("Count distinct",
             true,
             let
                CountDistinctRegion = Table.Group(DevOps, {"region"}, {{"CountDistinctRegion", each Table.RowCount(Table.Distinct(_)), type number}})
             in
                Table.RowCount(CountDistinctRegion) = 7 and AllRowsSatisfyCondition(CountDistinctRegion, each [CountDistinctRegion] = 300)
        ),
        Fact("AVG operation",
             53.82861816235487,
             let
                AverageMeasurePerHost = Table.Group(DevOps, {"hostname"}, {{"AverageMeasurePerHost", each List.Average([measure_value]), type number}})
             in
                AverageMeasurePerHost{[hostname="host-HGuD4"]}[AverageMeasurePerHost]
        ),
        Fact("Count",
             true,
             let
                CountHostName = Table.Group(DevOps, {"hostname"}, {{"CountHostname", each Table.RowCount(_), type number}})
             in
                Table.RowCount(CountHostName) = 105 and AllRowsSatisfyCondition(CountHostName, each [CountHostname] = 20)
        ),
        Fact("GroupBy",
             true,
             let
                GroupByRegion = Table.Group(DevOps, {"region"}, {{"GroupByRegion", each _, type table [hostname=text, az=text, measure_name=text, time=time, measure_value=number]}}),
                UsWest1 = GroupByRegion{[region="us-west-1"]}[GroupByRegion]
             in
                Table.RowCount(GroupByRegion) = 7 and Table.RowCount(UsWest1) = 300
        ),
        Fact("Max",
             99.573496855545727,
             let
                MaxMeasureByRegion = Table.Group(DevOps, {"region"}, {{"MaxMeasureByRegion", each List.Max([measure_value]), type number}})
             in
                MaxMeasureByRegion{[region="us-west-1"]}[MaxMeasureByRegion]
        ),
        Fact("Min",
             1.0125747631517084,
             let
                MinMeasureByRegion = Table.Group(DevOps, {"region"}, {{"MinMeasureByRegion", each List.Min([measure_value]), type number}})
             in
                MinMeasureByRegion{[region="us-west-1"]}[MinMeasureByRegion]
        ),
        Fact("Median",
             50.943772782463611,
             let
                MedianMeasureByRegion = Table.Group(DevOps, {"region"}, {{"MedianMeasureByRegion", each List.Median([measure_value]), type number}})
             in
                MedianMeasureByRegion{[region="us-west-1"]}[MedianMeasureByRegion]
        ),
        Fact("Sum",
             14808.27000449411,
             let
                SumMeasureByRegion = Table.Group(DevOps, {"region"}, {{"SumMeasureByRegion", each List.Sum([measure_value]), type number}})
             in
                SumMeasureByRegion{[region="us-west-1"]}[SumMeasureByRegion]
        ),

        // Random
        Fact("Replace Values",
             true,
             let
                Replaced = Table.ReplaceValue(DevOps, "us-west-1", "US-WEST-1", Replacer.ReplaceValue, {"region"}),
                uswest1 = Table.SelectRows(DevOps, each [region] = "us-west-1"),
                USWEST1AfterReplacement = Table.SelectRows(Replaced, each [region] = "US-WEST-1")
             in
                NRowsSatisfyCondition(Replaced, each [region] = "us-west-1", 0) and Table.RowCount(uswest1) = Table.RowCount(USWEST1AfterReplacement)
        ),
        Fact("Change Data Type",
             type text,
             let
                ChangedType = Table.TransformColumnTypes(DevOps, {{"measure_value", type text}})
             in
                Value.Type(ChangedType{0}[measure_value])
        ),
        Fact("Remove Duplicates",
             true,
             let
                RemovedDuplicateHostName = Table.Distinct(DevOps, {"hostname"}),
                IsHostNameUnique = (row) => NRowsSatisfyCondition(RemovedDuplicateHostName, each [hostname] = row[hostname], 1),
                AllDistinctHostNames = (table) => AllRowsSatisfyCondition(table, each IsHostNameUnique(_))
             in
                AllDistinctHostNames(RemovedDuplicateHostName)
        ),
        
        // Functions
        Fact("Earliest time",
             #datetime(2023, 3, 13, 15, 41, 43.478),
             List.Min(DevOps[time])
        ),
        Fact("Week of year",
             11,
             let
                WeekOfYear = Table.TransformColumns(DevOps, {{"time", Date.WeekOfYear, Int64.Type}})
             in
                WeekOfYear{0}[time]
        ),
        // DevOps{0} refers to the first column in DevOps, which is recorded below. 
        // | hostname	| az	     |  region	    | measure_name	        | time	            | measure_value::double
        // | host-13wwx	| ap-east-1a |	ap-east-1	| memory_utilization	| 2023-03-13 16:27	| 62.129919436193006
        Fact("Round Up",
             true,
             let
                RoundUp = Table.TransformColumns(DevOps, {{"measure_value", Number.RoundUp, Int64.Type}})
             in
                DevOps{0}[measure_value] > 62 and DevOps{0}[measure_value] < 63 and RoundUp{0}[measure_value] = 63
        ),
        Fact("Round Down",
             true,
             let
                RoundDown = Table.TransformColumns(DevOps, {{"measure_value", Number.RoundDown, Int64.Type}})
             in
                DevOps{0}[measure_value] > 62 and DevOps{0}[measure_value] < 63 and RoundDown{0}[measure_value] = 62
        ),
        // For functions like Sine, degree value is returned
        Fact("Sine",
             true,
             let
                Sine = Table.TransformColumns(DevOps, {{"measure_value", Number.Sin, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Sine{0}[measure_value] = -0.64569540805258363
        ),
        Fact("Cosine",
             true,
             let
                Cosine = Table.TransformColumns(DevOps, {{"measure_value", Number.Cos, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Cosine{0}[measure_value] = 0.76359507595309151
        ),
        Fact("Tangent",
             true,
             let
                Tangent = Table.TransformColumns(DevOps, {{"measure_value", Number.Tan, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Tangent{0}[measure_value] = -0.845599229731347
        ),
        Fact("Arcsine",
             true,
             let
                // We divide the measure_value column by 100 so we can get a possible value (between -1 and 1) to calculate arcsine.
                DivideByHundred = Table.TransformColumns(DevOps, {{"measure_value", each _ / 100, type number}}),
                Arcsine = Table.TransformColumns(DivideByHundred, {{"measure_value", Number.Asin, type number}})
             in
                DivideByHundred{0}[measure_value] = 0.62129919436193006 and Arcsine{0}[measure_value] = 0.67039965331419893
        ),
        Fact("Arccosine",
             true,
             let
                // We divide the measure_value column by 100 so we can get a possible value (between -1 and 1) to calculate arccosine.
                DivideByHundred = Table.TransformColumns(DevOps, {{"measure_value", each _ / 100, type number}}),
                Arccosine = Table.TransformColumns(DivideByHundred, {{"measure_value", Number.Acos, type number}})
             in
                DivideByHundred{0}[measure_value] = 0.62129919436193006 and Arccosine{0}[measure_value] = 0.90039667348069763
        ),
        Fact("Arctangent",
             true,
             let
                Arctangent = Table.TransformColumns(DevOps, {{"measure_value", Number.Atan, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Arctangent{0}[measure_value] = 1.5547024115034154
        ),
        Fact("Is Even",
             true,
             let
                RoundDown = Table.TransformColumns(DevOps, {{"measure_value", Number.RoundDown, Int64.Type}}),
                IsEven = Table.TransformColumns(RoundDown, {{"measure_value", Number.IsEven, type logical}})
             in
                RoundDown{0}[measure_value] = 62 and IsEven{0}[measure_value] = true
        ),
        Fact("Is Odd",
             true,
             let
                RoundDown = Table.TransformColumns(DevOps, {{"measure_value", Number.RoundDown, Int64.Type}}),
                IsOdd = Table.TransformColumns(RoundDown, {{"measure_value", Number.IsOdd, type logical}})
             in
                RoundDown{0}[measure_value] = 62 and IsOdd{0}[measure_value] = false
        ),
        Fact("Absolute Value",
             true,
             let
                Negative = Table.TransformColumns(DevOps, {{"measure_value", (n) => -n, type number}}),
                AbsValue = Table.TransformColumns(Negative, {{"measure_value", Number.Abs, type logical}})
             in
                Negative{0}[measure_value] = -62.129919436193006 and AbsValue{0}[measure_value] = 62.129919436193006
        ),
        Fact("Power",
             true,
             let
                Cube = Table.TransformColumns(DevOps, {{"measure_value", each Number.Power(_, 3), type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Cube{0}[measure_value] = 239829.37263623724
        ),
        Fact("Square Root",
             true,
             let
                SquareRoot = Table.TransformColumns(DevOps, {{"measure_value", Number.Sqrt, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and SquareRoot{0}[measure_value] = 7.8822534491218317
        ),
        Fact("Exponent",
             true,
             let
                // Divide the measure_value column by 10, then calculate exponent.
                DivideByTen = Table.TransformColumns(DevOps, {{"measure_value", each _ / 10, type number}}),
                Exponent = Table.TransformColumns(DivideByTen, {{"measure_value", Number.Exp, type number}})
             in
                DivideByTen{0}[measure_value] = 6.2129919436193006 and Exponent{0}[measure_value] = 499.19257523600612
        ),
        Fact("Natural Logarithm",
             true,
             let
                NatLog = Table.TransformColumns(DevOps, {{"measure_value", Number.Ln, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and NatLog{0}[measure_value] = 4.1292276673774015
        ),
        Fact("Factorial",
             true,
             let
                RoundUp = Table.TransformColumns(DevOps, {{"measure_value", Number.RoundUp, Int64.Type}}),
                Factorial = Table.TransformColumns(RoundUp, {{"measure_value", Number.Factorial, Int64.Type}})
             in
                RoundUp{0}[measure_value] = 63 and Factorial{0}[measure_value] = 1.9826083154044396E+87
        ),
        Fact("Add 100",
             true,
             let
                Add = Table.TransformColumns(DevOps, {{"measure_value", each _ + 100, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Add{0}[measure_value] = 162.129919436193006
        ),
        Fact("Multiply by 100",
             true,
             let
                Multiply = Table.TransformColumns(DevOps, {{"measure_value", each _ * 100, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Multiply{0}[measure_value] = 6212.9919436193006
        ),
        Fact("Subtract 100",
             true,
             let
                Subtract = Table.TransformColumns(DevOps, {{"measure_value", each _ - 100, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Subtract{0}[measure_value] = -37.870080563806994
        ),
        Fact("Divide by 100",
             true,
             let
                Divide = Table.TransformColumns(DevOps, {{"measure_value", each _ / 100, type number}})
             in
                DevOps{0}[measure_value] = 62.129919436193006 and Divide{0}[measure_value] = .62129919436193006
        ),
        Fact("Modulo",
             true,
             let
                // Multiply measure_value column by 100, round up the value, then calculate measure_value mod 5
                Multiply = Table.TransformColumns(DevOps, {{"measure_value", each _ * 100, type number}}),
                RoundUp = Table.TransformColumns(Multiply, {{"measure_value", Number.RoundUp, Int64.Type}}),
                Modulo = Table.TransformColumns(RoundUp, {{"measure_value", each Number.Mod(_, 5), Int64.Type}})
             in
                RoundUp{0}[measure_value] = 6213 and Modulo{0}[measure_value] = 3
        ),
        Fact("Extract",
             true,
             let
                Extract = Table.TransformColumns(DevOps, {{"region", each Text.AfterDelimiter(_, "ap-"), type text}})
             in
                DevOps{0}[region] = "ap-east-1" and Extract{0}[region] = "east-1"
        )
    },

    report = Facts.Summarize(facts)
][report];

/// COMMON UNIT TESTING CODE 
Fact = (_subject as text, _expected, _actual) as record =>
[   expected = try _expected,
    safeExpected = if expected[HasError] then "Expected : "& @ValueToText(expected[Error]) else expected[Value],
    actual = try _actual,
    safeActual = if actual[HasError] then "Actual : "& @ValueToText(actual[Error]) else actual[Value],
    attempt = try safeExpected = safeActual,
    result = if attempt[HasError] or not attempt[Value] then "Failure ⛔" else "Success ✓",
    resultOp = if result = "Success ✓" then " = " else " <> ",
    addendumEvalAttempt = if attempt[HasError] then @ValueToText(attempt[Error]) else "",
    addendumEvalExpected = try @ValueToText(safeExpected) otherwise "...",
    addendumEvalActual = try @ValueToText (safeActual) otherwise "...",
    fact =
    [   Result = result &" "& addendumEvalAttempt,
        Notes =_subject,
        Details = " ("& addendumEvalExpected & resultOp & addendumEvalActual &")"
    ]
][fact];

Facts = (_subject as text, _predicates as list) => List.Transform(_predicates, each Fact(_subject,_{0},_{1}));

Facts.Summarize = (_facts as list) as table =>
[   Fact.CountSuccesses = (count, i) =>
    [   result = try i[Result],
        sum = if result[HasError] or not Text.StartsWith(result[Value], "Success") then count else count + 1
    ][sum],
    passed = List.Accumulate(_facts, 0, Fact.CountSuccesses),
    total = List.Count(_facts),
    format = if passed = total then "All #{0} Passed !!! ✓" else "#{0} Passed ☺  #{1} Failed ☹",
    result = if passed = total then "Success" else "⛔",
    rate = Number.IntegerDivide(100*passed, total),
    header =
    [   Result = result,
        Notes = Text.Format(format, {passed, total-passed}),
        Details = Text.Format("#{0}% success rate", {rate})
    ],
    report = Table.FromRecords(List.Combine({{header},_facts}))
][report];

ValueToText = (value, optional depth) =>
    let
        List.TransformAndCombine = (list, transform, separator) => Text.Combine(List.Transform(list, transform), separator),

        Serialize.Binary =      (x) => "#binary(" & Serialize(Binary.ToList(x)) & ") ",

        Serialize.Function =    (x) => _serialize_function_param_type(
                                          Type.FunctionParameters(Value.Type(x)),
                                          Type.FunctionRequiredParameters(Value.Type(x)) ) &
                                       " as " &
                                       _serialize_function_return_type(Value.Type(x)) &
                                       " => (...) ",

        Serialize.List =        (x) => "{" & List.TransformAndCombine(x, Serialize, ", ") & "} ",

        Serialize.Record =      (x) => "[ " &
                                       List.TransformAndCombine(
                                            Record.FieldNames(x), 
                                            (item) => Serialize.Identifier(item) & " = " & Serialize(Record.Field(x, item)),
                                            ", ") &
                                       " ] ",

        Serialize.Table =       (x) => "#table( type " &
                                        _serialize_table_type(Value.Type(x)) &
                                        ", " &
                                        Serialize(Table.ToRows(x)) &
                                        ") ",
                                    
        Serialize.Identifier =  Expression.Identifier,

        Serialize.Type =        (x) => "type " & _serialize_typename(x),
                                    
                             
        _serialize_typename =    (x, optional funtype as logical) =>                        /* Optional parameter: Is this being used as part of a function signature? */
                                    let
                                        isFunctionType = (x as type) => try if Type.FunctionReturn(x) is type then true else false otherwise false,
                                        isTableType = (x as type) =>  try if Type.TableSchema(x) is table then true else false otherwise false,
                                        isRecordType = (x as type) => try if Type.ClosedRecord(x) is type then true else false otherwise false,
                                        isListType = (x as type) => try if Type.ListItem(x) is type then true else false otherwise false
                                    in
                                
                                        if funtype=null and isTableType(x) then _serialize_table_type(x) else
                                        if funtype=null and isListType(x) then "{ " & @_serialize_typename( Type.ListItem(x) ) & " }" else
                                        if funtype=null and isFunctionType(x) then "function " & _serialize_function_type(x) else
                                        if funtype=null and isRecordType(x) then _serialize_record_type(x) else
                                    
                                        if x = type any then "any" else
                                        let base = Type.NonNullable(x) in
                                          (if Type.IsNullable(x) then "nullable " else "") &       
                                          (if base = type anynonnull then "anynonnull" else                
                                          if base = type binary then "binary" else                
                                          if base = type date   then "date"   else
                                          if base = type datetime then "datetime" else
                                          if base = type datetimezone then "datetimezone" else
                                          if base = type duration then "duration" else
                                          if base = type logical then "logical" else
                                          if base = type none then "none" else
                                          if base = type null then "null" else
                                          if base = type number then "number" else
                                          if base = type text then "text" else 
                                          if base = type time then "time" else 
                                          if base = type type then "type" else 
                                      
                                          /* Abstract types: */
                                          if base = type function then "function" else
                                          if base = type table then "table" else
                                          if base = type record then "record" else
                                          if base = type list then "list" else
                                      
                                          "any /*Actually unknown type*/"),

        _serialize_table_type =     (x) => 
                                           let 
                                             schema = Type.TableSchema(x)
                                           in
                                             "table " &
                                             (if Table.IsEmpty(schema) then "" else 
                                                 "[" & List.TransformAndCombine(
                                                     Table.ToRecords(Table.Sort(schema,"Position")),
                                                     each Serialize.Identifier(_[Name]) & " = " & _[Kind],
                                                     ", ") &
                                                 "] "),

        _serialize_record_type =    (x) => 
                                            let flds = Type.RecordFields(x)
                                            in
                                                if Record.FieldCount(flds)=0 then "record" else
                                                    "[" & List.TransformAndCombine(
                                                        Record.FieldNames(flds),
                                                        (item) => Serialize.Identifier(item) & "=" & _serialize_typename(Record.Field(flds,item)[Type]),
                                                        ", ") & 
                                                    (if Type.IsOpenRecord(x) then ", ..." else "") &
                                                    "]",

        _serialize_function_type =  (x) => _serialize_function_param_type(
                                              Type.FunctionParameters(x),
                                              Type.FunctionRequiredParameters(x) ) &
                                            " as " &
                                            _serialize_function_return_type(x),
    
        _serialize_function_param_type = (t,n) => 
                                let
                                    funsig = Table.ToRecords(
                                        Table.TransformColumns(
                                            Table.AddIndexColumn( Record.ToTable( t ), "isOptional", 1 ),
                                            { "isOptional", (x)=> x>n } ) )
                                in
                                    "(" & 
                                    List.TransformAndCombine(
                                        funsig,
                                        (item)=>
                                            (if item[isOptional] then "optional " else "") &
                                            Serialize.Identifier(item[Name]) & " as " & _serialize_typename(item[Value], true),
                                        ", ") &
                                     ")",

        _serialize_function_return_type = (x) => _serialize_typename(Type.FunctionReturn(x), true), 

        Serialize = (x) as text => 
                           if x is binary       then try Serialize.Binary(x) otherwise "null /*serialize failed*/"        else 
                           if x is date         then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else 
                           if x is datetime     then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else 
                           if x is datetimezone then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else 
                           if x is duration     then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else 
                           if x is function     then try Serialize.Function(x) otherwise "null /*serialize failed*/"      else 
                           if x is list         then try Serialize.List(x) otherwise "null /*serialize failed*/"          else 
                           if x is logical      then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else
                           if x is null         then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else
                           if x is number       then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else
                           if x is record       then try Serialize.Record(x) otherwise "null /*serialize failed*/"        else 
                           if x is table        then try Serialize.Table(x) otherwise "null /*serialize failed*/"         else 
                           if x is text         then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else 
                           if x is time         then try Expression.Constant(x) otherwise "null /*serialize failed*/"     else 
                           if x is type         then try Serialize.Type(x) otherwise "null /*serialize failed*/"          else 
                           "[#_unable_to_serialize_#]"                     
    in
        try Serialize(value) otherwise "<serialization failed>";
