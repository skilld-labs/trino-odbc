# Power BI Overview

There are four methods for connecting Power BI to Amazon Trino.

1. [Power BI Desktop - ODBC](../microsoft-power-bi.md). Supports `AWS Profile`, `AWS IAM Credentials`, `Azure AD` and `Okta` authentication. ODBC data source must be created. Data is imported and must be refreshed for updates.
2. [Power BI Desktop - Custom Connector - Import Mode](./powerbi_custom_connector_import.md). Supports `AWS Profile` and `AWS IAM Credentials` only. ODBC data sources do not need to be created. Data is imported and must be refreshed for updates.
3. [Power BI Desktop - Custom Connector - Direct Query](./powerbi_custom_connector_direct_query.md). Supports `AWS Profile` and `AWS IAM Credentials` only. ODBC data sources do not need to be created. Queries are live and only return the results, all data does not need to be imported.
4. [Power BI Service](./powerbi_service.md). Using the `On-premises data gateway`, share reports created from Power BI Desktop on the web. 

[Custom Connector - Import Mode vs Direct Query Mode](https://social.technet.microsoft.com/wiki/contents/articles/53078.power-bi-import-mode-vs-directquery-mode.aspx)
