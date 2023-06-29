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

$ZipFile = '.\AmazonTimestreamConnector.zip'
$MezFile = '.\AmazonTimestreamConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous AmazonTimestreamConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating AmazonTimestreamConnector.m file

Copy-Item '.\AmazonTimestreamConnector\AmazonTimestreamConnector.pq' '.\AmazonTimestreamConnector\AmazonTimestreamConnector.m'

Write-Host Creating AmazonTimestreamConnector.zip file

compress-archive -path '.\AmazonTimestreamConnector\*.png', '.\AmazonTimestreamConnector\*.pqm', '.\AmazonTimestreamConnector\*.resx', '.\AmazonTimestreamConnector\AmazonTimestreamConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\AmazonTimestreamConnector\AmazonTimestreamConnector.m') {
    Write-Host Removing AmazonTimestreamConnector.m

    Remove-Item '.\AmazonTimestreamConnector\AmazonTimestreamConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous AmazonTimestreamConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating AmazonTimestreamConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
