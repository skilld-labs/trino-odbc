/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications Copyright Amazon.com, Inc. or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "trino/odbc/system/ui/dsn_configuration_window.h"

#include <Shlwapi.h>
#include <ShlObj_core.h>
#include <Windowsx.h>
#include <commctrl.h>

#include "trino/odbc/log.h"
#include "trino/odbc/log_level.h"
#include "trino/odbc/ignite_error.h"
#include "trino/odbc/authentication/auth_type.h"

#define TRIM_UTF8(str) utility::Trim(utility::ToUtf8(str))

namespace trino {
namespace odbc {
namespace system {
namespace ui {
DsnConfigurationWindow::DsnConfigurationWindow(Window* parent,
                                               config::Configuration& config)
    : CustomWindow(parent, L"TrinoConfigureDsn",
                   L"Configure Amazon Trino DSN"),
      width(450),
      height(425),
      nameEdit(),
      nameLabel(),
      nameBalloon(),
      endpointEdit(),
      endpointLabel(),
      regionEdit(),
      regionLabel(),
      tabs(),
      tabsGroupBox(),
      authTypeComboBox(),
      authTypeLabel(),
      profileNameEdit(),
      profileNameLabel(),
      connectionTimeoutEdit(),
      connectionTimeoutLabel(),
      reqTimeoutEdit(),
      reqTimeoutLabel(),
      maxRetryCountClientEdit(),
      maxRetryCountClientLabel(),
      maxConnectionsEdit(),
      maxConnectionsLabel(),
      maxConnectionsBalloon(),
      logLevelComboBox(),
      logLevelLabel(),
      logPathEdit(),
      logPathLabel(),
      browseButton(),
      testButton(),
      okButton(),
      cancelButton(),
      versionLabel(),
      config(config),
      accepted(false),
      created(false),
      shownNameBalloon(false),
      shownMaxConBalloon(false),
      preSel(TabIndex::Type::AUTHENTICATION) {
  // No-op.
}

DsnConfigurationWindow::~DsnConfigurationWindow() {
  // No-op.
}

void DsnConfigurationWindow::Create() {
  // Finding out parent position.
  RECT parentRect;
  GetWindowRect(parent->GetHandle(), &parentRect);

  // Positioning window to the center of parent window.
  const int posX =
      parentRect.left + (parentRect.right - parentRect.left - width) / 2;
  const int posY =
      parentRect.top + (parentRect.bottom - parentRect.top - height) / 2;

  RECT desiredRect = {posX, posY, posX + width, posY + height};
  AdjustWindowRect(&desiredRect,
                   WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, FALSE);

  Window::Create(WS_OVERLAPPED | WS_SYSMENU, desiredRect.left, desiredRect.top,
                 desiredRect.right - desiredRect.left,
                 desiredRect.bottom - desiredRect.top, 0);

  if (!handle) {
    std::stringstream buf;

    buf << "Can not create window, error code: " << GetLastError();

    throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
  }
}

void DsnConfigurationWindow::OnCreate() {
  int groupPosYLeft = MARGIN;
  int tabGroupPosY = groupPosYLeft;
  int groupSizeY = width - 2 * MARGIN;

  // create column group settings
  tabGroupPosY += +CreateBasicSettingsGroup(MARGIN, groupPosYLeft, groupSizeY);
  CreateAdvancedOptionsGroup(MARGIN, tabGroupPosY, groupSizeY);

  // Advance Authentication group is the tallest UI group, so the positions of
  // Ok button and Cancel button will be right below the end of Advance Auth
  groupPosYLeft +=
      tabGroupPosY
      + CreateAuthenticationSettingsGroup(MARGIN, tabGroupPosY, groupSizeY);
  CreateLogSettingsGroup(MARGIN, tabGroupPosY, groupSizeY);

  // Hide all tab windows except Authentication Window
  // Authentication window is the default
  ShowAdvancedOptions(false);
  ShowLogSettings(false);

  int cancelPosX = width - MARGIN - BUTTON_WIDTH;
  int okPosX = cancelPosX - INTERVAL - BUTTON_WIDTH;
  int testPosX = okPosX - INTERVAL - BUTTON_WIDTH;

  testButton = CreateButton(testPosX, groupPosYLeft, BUTTON_WIDTH,
                            BUTTON_HEIGHT, L"Test", ChildId::TEST_BUTTON);
  okButton = CreateButton(okPosX, groupPosYLeft, BUTTON_WIDTH, BUTTON_HEIGHT,
                          L"Ok", ChildId::OK_BUTTON);
  cancelButton = CreateButton(cancelPosX, groupPosYLeft, BUTTON_WIDTH,
                              BUTTON_HEIGHT, L"Cancel", ChildId::CANCEL_BUTTON);

  int versionPosX = MARGIN + INTERVAL;
  // re-use BUTTON_WIDTH because the version string has the same size
  // adjust y-position of version label to make it on the same level with the Ok
  // button
  versionLabel =
      CreateLabel(versionPosX, groupPosYLeft + 5, BUTTON_WIDTH, ROW_HEIGHT,
                  GetParsedDriverVersion(), ChildId::VERSION_LABEL);

  // check whether the required fields are filled. If not, Ok button is
  // disabled.
  created = true;
  okButton->SetEnabled(nameEdit->HasText());
}

void DsnConfigurationWindow::TestConnection() const {
  // Use a temporary configuration to test the connection.
  // Changes will not be committed until OK button is pressed.
  config::Configuration tempConfig;
  RetrieveParameters(tempConfig);
  std::vector< SQLWCHAR > dsnVec =
      utility::ToWCHARVector(tempConfig.ToConnectString());

  // Allocate an environment handle
  SQLHENV env = {};
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
  if (!SQL_SUCCEEDED(ret)) {
    MessageBox(handle, L"Unable to allocate Environment handle.", L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return;
  }

  // We want ODBC 3 support
  ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
                      reinterpret_cast< void* >(SQL_OV_ODBC3), 0);
  if (!SQL_SUCCEEDED(ret)) {
    MessageBox(handle, L"Unable to set ODBC version.", L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return;
  }

  // Allocate a connection handle
  SQLHDBC dbc = {};
  ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
  if (!SQL_SUCCEEDED(ret)) {
    MessageBox(handle, L"Unable to allocate Connection handle.", L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return;
  }

  // Test the connection.
  ret = SQLDriverConnect(dbc, nullptr, dsnVec.data(), dsnVec.size(), nullptr, 0,
                         nullptr, SQL_DRIVER_COMPLETE);
  if (!SQL_SUCCEEDED(ret)) {
    SQLWCHAR sqlState[7];
    SQLINTEGER nativeCode;
    SQLWCHAR errMessage[1024];
    SQLGetDiagRec(SQL_HANDLE_DBC, dbc, 1, sqlState, &nativeCode, errMessage,
                  sizeof(errMessage) / sizeof(SQLWCHAR), nullptr);
    std::stringstream buf;
    buf << "Connection failed: '" << utility::SqlWcharToString(errMessage)
        << "'";
    std::vector< SQLWCHAR > errMessageVec = utility::ToWCHARVector(buf.str());
    MessageBox(handle, errMessageVec.data(), L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return;
  }

  MessageBox(handle, L"Connection succeeded.", L"Success!",
             MB_ICONINFORMATION | MB_OK);

  // Cleanup
  SQLDisconnect(dbc);
  SQLFreeHandle(SQL_HANDLE_DBC, dbc);
  SQLFreeHandle(SQL_HANDLE_ENV, env);
}

std::wstring DsnConfigurationWindow::GetParsedDriverVersion(
    std::string driverVersion) {
  std::stringstream tmpStream;
  tmpStream << "V.";
  for (int i = 0; i < driverVersion.size(); i++) {
    switch (i) {
      case 7:
        if (driverVersion[i] != '0' || driverVersion[i - 1] != '0')
          tmpStream << driverVersion[i];

        break;
      case 8:
        if (driverVersion[i] != '0' || driverVersion[i - 1] != '0'
            || driverVersion[i - 2] != '0')
          tmpStream << driverVersion[i];

        break;
      case 4:
      case 9:
        // put at least digit (might be zero) in the sub-string
        tmpStream << driverVersion[i];

        break;
      // if the first digit in the sub-section is 0, do not include it in the
      // parsed string
      case 0:
      case 3:
      case 6:
        // continue if driverVersion[i] is not 0
        if (driverVersion[i] == '0')
          break;
      default:
        tmpStream << driverVersion[i];
    }
  }
  return utility::FromUtf8(tmpStream.str());
}

void DsnConfigurationWindow::ShowAdvanceAuth(bool visible) const {
  ShowWindow(authTypeComboBox->GetHandle(), visible);
  ShowWindow(authTypeLabel->GetHandle(), visible);
  if (visible) {
    // Show fields in Advance Authentication based on selection of authType
    OnAuthTypeChanged();
  } else {
    ShowWindow(profileNameEdit->GetHandle(), visible);
    ShowWindow(profileNameLabel->GetHandle(), visible);
  }
}

void DsnConfigurationWindow::ShowAdvancedOptions(bool visible) const {
  ShowWindow(connectionTimeoutEdit->GetHandle(), visible);
  ShowWindow(connectionTimeoutLabel->GetHandle(), visible);
  ShowWindow(reqTimeoutEdit->GetHandle(), visible);
  ShowWindow(reqTimeoutLabel->GetHandle(), visible);
  ShowWindow(maxRetryCountClientEdit->GetHandle(), visible);
  ShowWindow(maxRetryCountClientLabel->GetHandle(), visible);
  ShowWindow(maxConnectionsEdit->GetHandle(), visible);
  ShowWindow(maxConnectionsLabel->GetHandle(), visible);
}

void DsnConfigurationWindow::ShowLogSettings(bool visible) const {
  ShowWindow(logLevelComboBox->GetHandle(), visible);
  ShowWindow(logLevelLabel->GetHandle(), visible);
  ShowWindow(logPathEdit->GetHandle(), visible);
  ShowWindow(logPathLabel->GetHandle(), visible);
  ShowWindow(browseButton->GetHandle(), visible);
}

void DsnConfigurationWindow::OnSelChanged(TabIndex::Type idx) {
  if (preSel == idx) {
    LOG_DEBUG_MSG(
        "Tab index unchanged but OnSelChanged is triggered. Tab window will "
        "not change");
    return;
  }

  // hide previous tab selection
  switch (preSel) {
    case TabIndex::Type::AUTHENTICATION:
      ShowAdvanceAuth(false);

      break;
    case TabIndex::Type::ADVANCED_OPTIONS:
      ShowAdvancedOptions(false);

      break;
    case TabIndex::Type::LOG_SETTINGS:
      ShowLogSettings(false);

      break;
    default:
      // log wrong preSel value
      LOG_DEBUG_MSG("wrong preSel value: "
                    << preSel << ", this should not happen. Suggest to debug.");
  }

  // show current tab selection
  switch (idx) {
    case TabIndex::Type::AUTHENTICATION:
      ShowAdvanceAuth(true);

      break;
    case TabIndex::Type::ADVANCED_OPTIONS:
      ShowAdvancedOptions(true);

      break;
    case TabIndex::Type::LOG_SETTINGS:
      ShowLogSettings(true);

      break;
    default:
      // log wrong index value
      LOG_DEBUG_MSG("wrong idx value: "
                    << idx << ", this should not happen. Suggest to debug.");
  }

  // Hide all balloons when tabs switch
  Edit_HideBalloonTip(nameEdit->GetHandle());
  shownNameBalloon = false;
  Edit_HideBalloonTip(maxConnectionsEdit->GetHandle());
  shownMaxConBalloon = false;

  // save the previously selected index
  preSel = idx;
}

void DsnConfigurationWindow::OnAuthTypeChanged() const {
  // get value of authType
  AuthType::Type authType =
      static_cast< AuthType::Type >(authTypeComboBox->GetCBSelection());
  bool authTypePassword = authType == AuthType::Type::PASSWORD;
  // bool authTypeOauth2 = authType == AuthType::Type::OAUTH2;
  // bool authTypeKerberos = authType == AuthType::Type::KERBEROS;
  // bool authTypeCertificate = authType == AuthType::Type::CERTIFICATE;
  // bool authTypeJWT = authType == AuthType::Type::JWT;
  // bool authTypeHeader = authType == AuthType::Type::HEADER;

  // enable/disable PASSWORD fields
  profileNameEdit->SetEnabled(authTypePassword);

  // hide/show PASSWORD fields
  ShowWindow(profileNameEdit->GetHandle(), authTypePassword);
  ShowWindow(profileNameLabel->GetHandle(), authTypePassword);

}

void DsnConfigurationWindow::OnLogLevelChanged() const {
  std::wstring logLevelWStr;
  logLevelComboBox->GetText(logLevelWStr);
  if (LogLevel::FromString(TRIM_UTF8(logLevelWStr), LogLevel::Type::UNKNOWN)
      == LogLevel::Type::OFF) {
    logPathEdit->SetEnabled(false);
    browseButton->SetEnabled(false);
  } else {
    logPathEdit->SetEnabled(true);
    browseButton->SetEnabled(true);
  }
}

int DsnConfigurationWindow::CreateBasicSettingsGroup(int posX, int posY,
                                                     int sizeX) {
  enum { LABEL_WIDTH = 120 };

  int labelPosX = posX + INTERVAL;

  int tabSizeX = sizeX - 2 * INTERVAL;
  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY + INTERVAL;

  int checkBoxSize = sizeX - 2 * MARGIN;

  std::wstring wVal = utility::FromUtf8(config.GetDsn());
  nameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                          L"Data Source Name*:", ChildId::NAME_LABEL);
  nameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                        ChildId::NAME_EDIT);
  nameBalloon = CreateBalloon(L"Required Field",
                              L"DSN name is a required field.", TTI_ERROR);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetEndpoint());
  endpointLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Endpoint Override:", ChildId::ENDPOINT_LABEL);
  endpointEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                            ChildId::ENDPOINT_EDIT);
  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetRegion());
  regionLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                            L"Region:", ChildId::REGION_LABEL);
  regionEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                          ChildId::REGION_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  tabs = CreateTab(labelPosX, rowPos, tabSizeX, ROW_HEIGHT, L"Tabs",
                   ChildId::TABS);

  tabs->AddTab(TabIndex::Type::AUTHENTICATION, L"Authentication");
  tabs->AddTab(TabIndex::Type::ADVANCED_OPTIONS, L"Advanced Options");
  tabs->AddTab(TabIndex::Type::LOG_SETTINGS, L"Logging Options");

  tabsGroupBox = CreateGroupBox(posX, rowPos + 15, sizeX, 260, L"",
                                ChildId::TABS_GROUP_BOX);

  rowPos += INTERVAL + ROW_HEIGHT;

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateAuthenticationSettingsGroup(int posX,
                                                              int posY,
                                                              int sizeX) {
  enum { LABEL_WIDTH = 120 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY;

  AuthType::Type authType = config.GetAuthType();
  authTypeLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Auth Type:", ChildId::AUTH_TYPE_LABEL);
  authTypeComboBox = CreateComboBox(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                    L"", ChildId::AUTH_TYPE_COMBO_BOX);

  // the order of add string needs to match the definition of the auth_type.h
  // file
  for (int i = 0; i <= 3; i++) {
    authTypeComboBox->AddString(
        AuthType::ToCBString(static_cast< AuthType::Type >(i)));
  }

  authTypeComboBox->SetCBSelection(
      static_cast< int >(authType));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  int authTypeRowPos = rowPos;

  wVal = utility::FromUtf8(config.GetProfileName());
  profileNameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                 L"Profile Name:", ChildId::PROFILE_NAME_LABEL);
  profileNameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                               ChildId::PROFILE_NAME_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  rowPos = authTypeRowPos;

  OnAuthTypeChanged();

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateAdvancedOptionsGroup(int posX, int posY,
                                                       int sizeX) {
  enum { LABEL_WIDTH = 120 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY;

  std::wstring wVal = std::to_wstring(config.GetConnectionTimeout());
  connectionTimeoutLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                                       ROW_HEIGHT, L"Connection Timeout (ms):",
                                       ChildId::CONNECTION_TIMEOUT_LABEL);
  connectionTimeoutEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                 ChildId::CONNECTION_TIMEOUT_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetReqTimeout());
  reqTimeoutLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                  L"Request Timeout (ms):", ChildId::REQ_TIMEOUT_LABEL);
  reqTimeoutEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                              ChildId::REQ_TIMEOUT_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetMaxRetryCountClient());
  maxRetryCountClientLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                                         ROW_HEIGHT, L"Max retry count client:",
                                         ChildId::MAX_RETRY_COUNT_CLIENT_LABEL);
  maxRetryCountClientEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                 ChildId::MAX_RETRY_COUNT_CLIENT_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetMaxConnections());
  maxConnectionsLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                  L"Max connections:", ChildId::MAX_CONNECTIONS_LABEL);
  maxConnectionsEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                                  ChildId::MAX_CONNECTIONS_EDIT, ES_NUMBER);
  maxConnectionsBalloon = CreateBalloon(
      L"Positive Number Only",
      L"Number of connections must be a positive number.", TTI_ERROR);

  rowPos += INTERVAL + ROW_HEIGHT;

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateLogSettingsGroup(int posX, int posY,
                                                   int sizeX) {
  enum { LABEL_WIDTH = 120 };

  int labelPosX = posX + INTERVAL;
  int pathSizeX = sizeX - 3 * INTERVAL - BUTTON_WIDTH;
  int comboSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int comboPosX = labelPosX + LABEL_WIDTH + INTERVAL;
  int editPosX = labelPosX;

  int rowPos = posY;

  LogLevel::Type logLevel = config.GetLogLevel();

  logLevelLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Log Level:", ChildId::LOG_LEVEL_LABEL);
  logLevelComboBox = CreateComboBox(comboPosX, rowPos, comboSizeX, ROW_HEIGHT,
                                    L"", ChildId::LOG_LEVEL_COMBO_BOX);

  // the order of add string needs to match the definition of the log_level.h
  // file
  int logLevelUpperLimit = static_cast< int >(LogLevel::Type::UNKNOWN);
  for (int i = 0; i < logLevelUpperLimit; i++) {
    logLevelComboBox->AddString(
        LogLevel::ToCBString(static_cast< LogLevel::Type >(i)));
  }

  logLevelComboBox->SetCBSelection(
      static_cast< int >(logLevel));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring wVal = utility::FromUtf8(config.GetLogPath());
  logPathLabel = CreateLabel(
      labelPosX, rowPos, pathSizeX, ROW_HEIGHT * 2,
      L"Log Path:\n(the log file name format is trino_odbc_YYYYMMDD.log)",
      ChildId::LOG_PATH_LABEL);

  rowPos += INTERVAL * 2 + ROW_HEIGHT;

  logPathEdit = CreateEdit(editPosX, rowPos, pathSizeX, ROW_HEIGHT, wVal,
                           ChildId::LOG_PATH_EDIT);

  // Slightly adjust the browse button position to align with the log path field
  // on the same level
  browseButton =
      CreateButton(editPosX + pathSizeX + INTERVAL, rowPos - 2, BUTTON_WIDTH,
                   BUTTON_HEIGHT, L"Browse", ChildId::BROWSE_BUTTON);

  rowPos += INTERVAL + ROW_HEIGHT;

  OnLogLevelChanged();

  return rowPos - posY;
}

// Callback function to set the initial path.
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam,
                                LPARAM lpData) {
  switch (uMsg) {
    case BFFM_INITIALIZED: {
      if (lpData != NULL)
        SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
    } break;
  }
  return 0;
}

bool DsnConfigurationWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_COMMAND: {
      switch (LOWORD(wParam)) {
        case ChildId::TEST_BUTTON: {
          TestConnection();

          break;
        }

        case ChildId::OK_BUTTON: {
          try {
            RetrieveParameters(config);

            accepted = true;

            PostMessage(GetHandle(), WM_CLOSE, 0, 0);
          } catch (IgniteError& err) {
            std::wstring errWText = utility::FromUtf8(err.GetText());
            MessageBox(NULL, errWText.c_str(), L"Error!",
                       MB_ICONEXCLAMATION | MB_OK);
          }

          break;
        }

        case IDCANCEL:
        case ChildId::CANCEL_BUTTON: {
          PostMessage(GetHandle(), WM_CLOSE, 0, 0);

          break;
        }

        case ChildId::NAME_EDIT: {
          // Check if window has been created.
          if (created) {
            okButton->SetEnabled(nameEdit->HasText());

            if (!shownNameBalloon && !nameEdit->HasText()) {
              Edit_ShowBalloonTip(nameEdit->GetHandle(), nameBalloon.get());
              shownNameBalloon = true;
            } else {
              Edit_HideBalloonTip(nameEdit->GetHandle());
              shownNameBalloon = false;
            }
          }
          break;
        }

        case ChildId::MAX_CONNECTIONS_EDIT: {
          if (created) {
            std::wstring maxConWStr;
            maxConnectionsEdit->GetText(maxConWStr);

            std::string maxConStr = TRIM_UTF8(maxConWStr);

            int16_t maxCon =
                trino::odbc::common::LexicalCast< int16_t >(maxConStr);

            if (!shownMaxConBalloon && maxCon <= 0) {
              Edit_ShowBalloonTip(maxConnectionsEdit->GetHandle(),
                                  maxConnectionsBalloon.get());
              shownMaxConBalloon = true;
            } else {
              Edit_HideBalloonTip(maxConnectionsEdit->GetHandle());
              shownMaxConBalloon = false;
            }
          }
          break;
        }

        case ChildId::AUTH_TYPE_COMBO_BOX: {
          OnAuthTypeChanged();

          break;
        }

        case ChildId::LOG_LEVEL_COMBO_BOX: {
          OnLogLevelChanged();

          break;
        }

        case ChildId::BROWSE_BUTTON: {
          std::wstring initLogPath;
          logPathEdit->GetText(initLogPath);
          std::unique_ptr< BROWSEINFO > bi(std::make_unique< BROWSEINFO >());
          bi->lpszTitle = L"Choose log file target directory:";
          bi->ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
          bi->hwndOwner = browseButton->GetHandle();
          bi->lpfn = BrowseCallbackProc;
          bi->lParam = reinterpret_cast< LPARAM >(initLogPath.c_str());

          const LPITEMIDLIST& pidl = SHBrowseForFolder(bi.get());

          if (pidl != nullptr) {
            // get the name of the folder and put it in the log path field
            wchar_t logPath[_MAX_PATH];
            SHGetPathFromIDList(pidl, logPath);
            logPathEdit->SetText(static_cast< std::wstring >(logPath));
          }

          break;
        }

        default:
          return false;
      }

      break;
    }

    case WM_DESTROY: {
      PostQuitMessage(accepted ? Result::OK : Result::CANCEL);

      break;
    }

    case WM_NOTIFY: {
      switch (LOWORD(wParam)) {
        case ChildId::TABS: {
          LOG_DEBUG_MSG("current Tab selection index (without cast): "
                        << tabs->GetTabSelection());

          TabIndex::Type curSel =
              static_cast< TabIndex::Type >(tabs->GetTabSelection());

          LOG_DEBUG_MSG("current Tab selection index (with cast): " << curSel);

          OnSelChanged(curSel);
          break;
        }

        default:
          return false;
      }

      break;
    }

    default:
      return false;
  }

  return true;
}

void DsnConfigurationWindow::RetrieveParameters(
    config::Configuration& cfg) const {
  // Intentioanlly retrieve log parameters first
  RetrieveLogParameters(cfg);
  RetrieveBasicParameters(cfg);
  RetrieveBasicAuthParameters(cfg);
  RetrieveAdvanceAuthParameters(cfg);
  RetrieveConnectionParameters(cfg);
}

void DsnConfigurationWindow::RetrieveBasicParameters(
    config::Configuration& cfg) const {
  std::wstring dsnWStr;
  std::wstring endpointWStr;
  std::wstring regionWStr;

  nameEdit->GetText(dsnWStr);
  endpointEdit->GetText(endpointWStr);
  regionEdit->GetText(regionWStr);

  std::string dsnStr = TRIM_UTF8(dsnWStr);
  std::string endpointStr = TRIM_UTF8(endpointWStr);
  std::string regionStr = TRIM_UTF8(regionWStr);

  cfg.SetDsn(dsnStr);
  cfg.SetEndpoint(endpointStr);
  cfg.SetRegion(regionStr);

  LOG_INFO_MSG("Retrieving arguments:");
  LOG_INFO_MSG("DSN:                             " << dsnStr);
  LOG_INFO_MSG("Endpoint:    " << endpointStr);
  LOG_INFO_MSG("Region:      " << regionStr);
}

void DsnConfigurationWindow::RetrieveBasicAuthParameters(
    config::Configuration& cfg) const {
  std::wstring profileNameWStr;

  profileNameEdit->GetText(profileNameWStr);

  std::string profileNameStr = TRIM_UTF8(profileNameWStr);

  cfg.SetProfileName(profileNameStr);

  LOG_INFO_MSG("Retrieving arguments:");
  LOG_INFO_MSG("Profile Name: " << profileNameStr);
  // username and password intentionally not logged for security reasons
}

void DsnConfigurationWindow::RetrieveAdvanceAuthParameters(
    config::Configuration& cfg) const {
  std::wstring authTypeWStr;

  authTypeComboBox->GetText(authTypeWStr);  // auth type comboBox string
                                            // retrieved for debugging purpose
  AuthType::Type authType =
      static_cast< AuthType::Type >(authTypeComboBox->GetCBSelection());

  cfg.SetAuthType(authType);

  LOG_INFO_MSG("Auth Type:    " << AuthType::ToString(authType));
  LOG_DEBUG_MSG("Auth Type string from combobox" << TRIM_UTF8(authTypeWStr));
  LOG_DEBUG_MSG("AuthType::Type authType: " << static_cast< int >(authType));
}

void DsnConfigurationWindow::RetrieveConnectionParameters(
    config::Configuration& cfg) const {
  std::wstring connectionTimeoutWStr;
  std::wstring reqTimeoutWStr;
  std::wstring maxRetryCountWStr;
  std::wstring maxConWStr;

  connectionTimeoutEdit->GetText(connectionTimeoutWStr);
  reqTimeoutEdit->GetText(reqTimeoutWStr);
  maxRetryCountClientEdit->GetText(maxRetryCountWStr);
  maxConnectionsEdit->GetText(maxConWStr);

  std::string connectionTimeoutStr = TRIM_UTF8(connectionTimeoutWStr);
  std::string reqTimeoutStr = TRIM_UTF8(reqTimeoutWStr);
  std::string maxRetryCountStr = TRIM_UTF8(maxRetryCountWStr);
  std::string maxConStr = TRIM_UTF8(maxConWStr);

  int32_t connectionTimeout =
      connectionTimeoutStr.empty()
          ? 0
          : trino::odbc::common::LexicalCast< int32_t >(
              connectionTimeoutStr);
  if (connectionTimeout < 0) {
    connectionTimeout = config::Configuration::DefaultValue::connectionTimeout;
  }

  int32_t reqTimeout =
      reqTimeoutStr.empty()
          ? 0
          : trino::odbc::common::LexicalCast< int32_t >(reqTimeoutStr);
  if (reqTimeout < 0) {
    reqTimeout = config::Configuration::DefaultValue::reqTimeout;
  }

  int32_t maxRetryCountClient =
      maxRetryCountStr.empty()
          ? 0
          : trino::odbc::common::LexicalCast< int32_t >(maxRetryCountStr);
  if (maxRetryCountClient < 0) {
    maxRetryCountClient =
        config::Configuration::DefaultValue::maxRetryCountClient;
  }

  int32_t maxCon =
      maxConStr.empty()
          ? 0
          : trino::odbc::common::LexicalCast< int32_t >(maxConStr);

  if (maxCon <= 0)
    throw IgniteError(
        IgniteError::IGNITE_ERR_GENERIC,
        "[Max Connections] Number of connections must be a positive number.");

  cfg.SetConnectionTimeout(connectionTimeout);
  cfg.SetReqTimeout(reqTimeout);
  cfg.SetMaxRetryCountClient(maxRetryCountClient);
  cfg.SetMaxConnections(maxCon);

  LOG_INFO_MSG("Connection timeout (ms):  " << connectionTimeout);
  LOG_INFO_MSG("Request timeout (ms): " << reqTimeout);
  LOG_INFO_MSG("Max retry count client:      " << maxRetryCountClient);
  LOG_INFO_MSG("Max connections:      " << maxCon);
}

// RetrieveLogParameters is a special case. We want to get the log level and
// path as soon as possible. If user set log level to OFF, then nothing should
// be logged. Therefore, the LOG_MSG calls are after log level and log path are
// set.
void DsnConfigurationWindow::RetrieveLogParameters(
    config::Configuration& cfg) const {
  std::wstring logLevelWStr;
  std::wstring logPathWStr;

  logLevelComboBox->GetText(logLevelWStr);
  logPathEdit->GetText(logPathWStr);

  std::string logLevelStr = TRIM_UTF8(logLevelWStr);
  std::string logPathStr = TRIM_UTF8(logPathWStr);

  LogLevel::Type logLevel =
      static_cast< LogLevel::Type >(logLevelComboBox->GetCBSelection());

  cfg.SetLogLevel(logLevel);
  cfg.SetLogPath(logPathStr);

  LOG_INFO_MSG("Log level:    " << logLevelStr);
  LOG_DEBUG_MSG("LogLevel Type string from combobox" << logLevelStr);
  LOG_DEBUG_MSG("LogLevel::Type logLevel: " << static_cast< int >(logLevel));
  LOG_INFO_MSG("Log path:     " << logPathStr);
}
}  // namespace ui
}  // namespace system
}  // namespace odbc
}  // namespace trino
