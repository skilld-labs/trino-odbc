# 
#    Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
# 
#    Licensed under the Apache License, Version 2.0 (the "License").
#    You may not use this file except in compliance with the License.
#    A copy of the License is located at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#    or in the "license" file accompanying this file. This file is distributed
#    on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#    express or implied. See the License for the specific language governing
#    permissions and limitations under the License.
# 

$scriptPath = Split-Path -parent $PSCommandPath

# . $scriptPath\functions_win.ps1

# Sign a single file
function Invoke-SignFile {
    [OutputType([Boolean])]
    Param(
        # The path to the file to sign
        [Parameter(Mandatory=$true)]
        [string]$SourcePath,
        # The path to the signed file
        [Parameter(Mandatory=$true)]
        [string]$TargetPath,
        # The name of the unsigned AWS bucket
        [Parameter(Mandatory=$true)]
        [string]$AwsUnsignedBucket,
        [Parameter(Mandatory=$true)]
        # The name of the signed AWS bucket
        [string]$AwsSignedBucket,
        # The name of the AWS key
        [Parameter(Mandatory=$true)]
        [string]$AwsKey,
        [Parameter(Mandatory=$false)]
        [bool]$AsMockResponse=$false
    )

    Write-Host "Will attempt to sign $SourcePath"
    
    # Remember to install 'jq' dependency before
    if ($AsMockResponse) {
        Copy-Item $SourcePath $TargetPath
        return $true
    }

    # Upload unsigned .exe to S3 Bucket
    Write-Host "Obtaining version id and uploading unsigned .exe to S3 Bucket"
    $versionId = $( aws s3api put-object --bucket $AwsUnsignedBucket --key $AwsKey --body $SourcePath --acl bucket-owner-full-control | jq '.VersionId' )
    $jobId = ""

    if ([string]::IsNullOrEmpty($versionId)) {
        Write-Host "Failed to PUT unsigned file in bucket."
        return $false
    }

    # Attempt to get Job ID from bucket tagging, will retry up to 3 times before exiting with a failure code.
    # Will sleep for 5 seconds between retries.
    Write-Host "Attempt to get Job ID from bucket tagging, will retry up to 3 times before exiting with a failure code."
    for ( $i = 0; $i -lt 3; $i++ ) {
        # Get job ID
        $id=$( aws s3api get-object-tagging --bucket $AwsUnsignedBucket --key $AwsKey --version-id $versionId | jq -r '.TagSet[0].Value' )
        if ( $id -ne "null" ) {
            $jobId = $id
            break
        }

        Write-Host "Will sleep for 5 seconds between retries."
        Start-Sleep -Seconds 5
    }

    if ( $jobId -eq "" ) {
        Write-Host "Exiting because unable to retrieve job ID"
        return $false
    }

    Write-Host "Job ID: " $jobId

    # Poll signed S3 bucket to see if the signed artifact is there, will retry up to 3 times before exiting with a failure code.
    # Will sleep for 5 seconds between retries.
    Write-Host "Poll signed S3 bucket to see if the signed artifact is there"
    for ( $i = 0; $i -lt 3; $i++ ) {
        aws s3api wait object-exists --bucket $AwsSignedBucket --key $AwsKey-$jobId
        # Check if successful
        if ( $LASTEXITCODE -eq 0 ) {
            break
        }
    
        Write-Host "Will sleep for 5 seconds between retries."
        Start-Sleep -Seconds 5
    }

    # Get signed EXE from S3
    Write-Host "Get signed EXE from S3 to $TargetPath"
    aws s3api get-object --bucket $AwsSignedBucket --key $AwsKey-$jobId $TargetPath

    Write-Host "Signing completed"
    return $true
}

# Sign the installer file for a given architecture and ODBC version
function Invoke-SignInstaller {
    [OutputType([Boolean])]
    Param(
        # The path to the build directory.
        [Parameter(Mandatory=$true)]
        [string]$BuildDir,
        # The architecture name.
        [Parameter(Mandatory=$true)]
        [string]$Architecture,
        # The ODBC version.
        [Parameter(Mandatory=$true)]
        [string]$OdbcVersion,
        # The name of the unsigned AWS bucket
        [Parameter(Mandatory=$true)]
        [string]$AwsUnsignedBucket,
        [Parameter(Mandatory=$true)]
        # The name of the signed AWS bucket
        [string]$AwsSignedBucket,
        # The name of the AWS key
        [Parameter(Mandatory=$true)]
        [string]$AwsKey,
        [Parameter(Mandatory=$false)]
        [bool]$AsMockResponse=$false
    )

    $unsignedInstallerPath=$(Join-Path $BuildDir "trino-odbc-installer-$Architecture-$OdbcVersion.exe")
    $signedInstallerPath=$(Join-Path $BuildDir "trino-odbc-installer-$Architecture-$OdbcVersion-signed.exe")
    $unsignedEnginePath=$(Join-Path $BuildDir "trino-odbc-installer-$Architecture-$OdbcVersion-engine.exe")
    $signedEnginePath=$(Join-Path $BuildDir "trino-odbc-installer-$Architecture-$OdbcVersion-engine-signed.exe")

    # Extract unsigned engine
    Write-Host "Extracting unsigned engine."
    insignia.exe -ib "$unsignedInstallerPath" -o "$unsignedEnginePath"
    # Signing the engine
    Write-Host "Signing engine."
    if ( -not (Invoke-SignFile $unsignedEnginePath $signedEnginePath $AwsUnsignedBucket $AwsSignedBucket $AwsKey -AsMockResponse $AsMockResponse) ) {
        Write-Host "Failed to sign engine file."
        return $false
    }
    
    # Re-attach the signed engine
    Write-Host "Attaching signed engine."
    insignia.exe -ab "$signedEnginePath" "$unsignedInstallerPath" -o "$unsignedInstallerPath"

    # Sign the installer
    Write-Host "Signing the installer."
    if ( -not (Invoke-SignFile $unsignedInstallerPath $signedInstallerPath $AwsUnsignedBucket $AwsSignedBucket $AwsKey -AsMockResponse $AsMockResponse) ) {
        Write-Host "Failed to sign installer file."
        return $false
    }

    Write-Host "Removing unused engine files."
    if (Test-Path $unsignedEnginePath) {
        Remove-Item $unsignedEnginePath
    }
    if (Test-Path $signedEnginePath) {
        Remove-Item $signedEnginePath
    }

    # Remove unsigned installer and remove "-signed" in signed installer name
    Write-Host "Removing unsigned executable."
    Remove-Item -Path $unsignedInstallerPath
    Rename-Item -Path $signedInstallerPath -NewName "trino-odbc-installer-$Architecture-$OdbcVersion.exe"

    return $true
}
