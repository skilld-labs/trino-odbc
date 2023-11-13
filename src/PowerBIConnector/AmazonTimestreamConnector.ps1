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

$ZipFile = '.\AmazonTrinoConnector.zip'
$MezFile = '.\AmazonTrinoConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous AmazonTrinoConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating AmazonTrinoConnector.m file

Copy-Item '.\AmazonTrinoConnector\AmazonTrinoConnector.pq' '.\AmazonTrinoConnector\AmazonTrinoConnector.m'

Write-Host Creating AmazonTrinoConnector.zip file

compress-archive -path '.\AmazonTrinoConnector\*.png', '.\AmazonTrinoConnector\*.pqm', '.\AmazonTrinoConnector\*.resx', '.\AmazonTrinoConnector\AmazonTrinoConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\AmazonTrinoConnector\AmazonTrinoConnector.m') {
    Write-Host Removing AmazonTrinoConnector.m

    Remove-Item '.\AmazonTrinoConnector\AmazonTrinoConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous AmazonTrinoConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating AmazonTrinoConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
