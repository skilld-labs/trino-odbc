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

$ZipFile = '.\AmazonTrinoAADConnector.zip'
$MezFile = '.\AmazonTrinoAADConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous AmazonTrinoAADConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating AmazonTrinoAADConnector.m file

Copy-Item '.\AmazonTrinoAADConnector\AmazonTrinoAADConnector.pq' '.\AmazonTrinoAADConnector\AmazonTrinoAADConnector.m'

Write-Host Creating AmazonTrinoAADConnector.zip file

compress-archive -path '.\AmazonTrinoAADConnector\*.png', '.\AmazonTrinoAADConnector\*.pqm', '.\AmazonTrinoAADConnector\*.resx', '.\AmazonTrinoAADConnector\AmazonTrinoAADConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\AmazonTrinoAADConnector\AmazonTrinoAADConnector.m') {
    Write-Host Removing AmazonTrinoAADConnector.m

    Remove-Item '.\AmazonTrinoAADConnector\AmazonTrinoAADConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous AmazonTrinoAADConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating AmazonTrinoAADConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
