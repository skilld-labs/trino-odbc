# Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.

# retrieve access key id and secret key from environment variables
$REPO_ROOT = [System.Environment]::GetEnvironmentVariable('REPOSITORY_ROOT')
$ENV_ACCESS_ID = [System.Environment]::GetEnvironmentVariable('AWS_ACCESS_KEY_ID')
$ENV_SECRET_KEY = [System.Environment]::GetEnvironmentVariable('AWS_SECRET_ACCESS_KEY')

If ($ENV_ACCESS_ID -eq $null -or $ENV_ACCESS_ID -eq "") {
    Write-Host "Environment variable AWS_ACCESS_KEY_ID is null or empty."
    $MISSING_VARS = 1
}

If ($ENV_SECRET_KEY -eq $null -or $ENV_SECRET_KEY -eq "") {
    Write-Host "Environment variable AWS_SECRET_ACCESS_KEY is null or empty."
    $MISSING_VARS = 1
}

If ($REPO_ROOT -eq $null -or $REPO_ROOT -eq "") {
    Write-Host "Environment variable REPOSITORY_ROOT is null or empty."
    $MISSING_VARS = 1
}

If ($MISSING_VARS -eq 1) {
    Write-Host "Missing environment variables, please set them accordingly. Exiting."
    Exit 1
}

echo "[test-profile]" > $REPO_ROOT\src\tests\input\credentials
echo "aws_access_key_id = $ENV_ACCESS_ID" >> $REPO_ROOT\src\tests\input\credentials
echo "aws_secret_access_key = $ENV_SECRET_KEY" >> $REPO_ROOT\src\tests\input\credentials

echo "[incomplete-profile]" >> $REPO_ROOT\src\tests\input\credentials
echo "aws_access_key_id = $ENV_ACCESS_ID" >> $REPO_ROOT\src\tests\input\credentials

# Convert CRLFs to LFs 
$original_file = "$REPO_ROOT\src\tests\input\credentials"
(Get-Content $original_file) | Foreach-Object {
$_ -replace "`r`n",'`n' } | 
Set-Content "$REPO_ROOT\src\tests\input\credentials" -Force
