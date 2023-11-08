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

#ifndef _TIMESTREAM_ODBC_SYSTEM_UI_DSN_CONFIGURATION_WINDOW
#define _TIMESTREAM_ODBC_SYSTEM_UI_DSN_CONFIGURATION_WINDOW

#include "timestream/odbc/config/configuration.h"
#include "timestream/odbc/config/connection_info.h"
#include "timestream/odbc/system/ui/custom_window.h"

namespace timestream {
namespace odbc {
namespace system {
namespace ui {
/**
 * DSN configuration window class.
 */
class DsnConfigurationWindow : public CustomWindow {
  /**
   * Children windows ids.
   */
  /*$*/
  struct ChildId {
    enum Type {
      NAME_EDIT = 100,
      NAME_LABEL,
      ENDPOINT_EDIT,
      ENDPOINT_LABEL,
      REGION_EDIT,
      REGION_LABEL,
      TABS,
      TABS_GROUP_BOX,
      AUTH_TYPE_LABEL,
      AUTH_TYPE_COMBO_BOX,
      ACCESS_KEY_ID_LABEL,
      ACCESS_KEY_ID_EDIT,
      SECRET_ACCESS_KEY_LABEL,
      SECRET_ACCESS_KEY_EDIT,
      SESSION_TOKEN_LABEL,
      SESSION_TOKEN_EDIT,
      PROFILE_NAME_EDIT,
      PROFILE_NAME_LABEL,
      ROLE_ARN_EDIT,
      ROLE_ARN_LABEL,
      IDP_USER_NAME_EDIT,
      IDP_USER_NAME_LABEL,
      IDP_PASSWORD_EDIT,
      IDP_PASSWORD_LABEL,
      IDP_ARN_EDIT,
      IDP_ARN_LABEL,
      IDP_HOST_EDIT,
      IDP_HOST_LABEL,
      OKTA_APP_ID_EDIT, /*$*/
      OKTA_APP_ID_LABEL,
      AAD_APP_ID_EDIT,
      AAD_APP_ID_LABEL,
      AAD_CLIENT_SECRET_EDIT,
      AAD_CLIENT_SECRET_LABEL,
      AAD_TENANT_EDIT,
      AAD_TENANT_LABEL,
      CONNECTION_TIMEOUT_EDIT,
      CONNECTION_TIMEOUT_LABEL,
      REQ_TIMEOUT_EDIT,
      REQ_TIMEOUT_LABEL,
      MAX_RETRY_COUNT_CLIENT_EDIT,
      MAX_RETRY_COUNT_CLIENT_LABEL,
      MAX_CONNECTIONS_EDIT,
      MAX_CONNECTIONS_LABEL,
      LOG_LEVEL_COMBO_BOX,
      LOG_LEVEL_LABEL,
      LOG_PATH_EDIT,
      LOG_PATH_LABEL,
      BROWSE_BUTTON,
      TEST_BUTTON,
      OK_BUTTON,
      CANCEL_BUTTON,
      VERSION_LABEL
    };
  };

  // Window margin size.
  enum { MARGIN = 10 };

  // Standard interval between UI elements.
  enum { INTERVAL = 10 };

  // Standard row height.
  enum { ROW_HEIGHT = 20 };

  // Standard button width.
  enum { BUTTON_WIDTH = 80 };

  // Standard button height.
  enum { BUTTON_HEIGHT = 25 };

  // Tab indices
  struct TabIndex {
    enum Type { AUTHENTICATION, ADVANCED_OPTIONS, LOG_SETTINGS };
  };

 public:
  /**
   * Constructor.
   *
   * @param parent Parent window handle.
   */
  explicit DsnConfigurationWindow(Window* parent,
                                  config::Configuration& config);

  /**
   * Destructor.
   */
  virtual ~DsnConfigurationWindow();

  /**
   * Create window in the center of the parent window.
   */
  void Create();

  /**
   * @copedoc timestream::odbc::system::ui::CustomWindow::OnCreate
   */
  virtual void OnCreate();

  /**
   * @copedoc timestream::odbc::system::ui::CustomWindow::OnMessage
   */
  virtual bool OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  /*
   * Returns the parsed Driver Version in the format V.XX.XX.XXXX.
   * Leading zeros will not be included.
   *
   * @param driverVersion Driver Version number to be parsed.
   * @return str Parsed Driver Version.
   */
  static std::wstring GetParsedDriverVersion(
      std::string driverVersion = utility::GetFormatedDriverVersion());

 private:
  IGNITE_NO_COPY_ASSIGNMENT(DsnConfigurationWindow)

  /**
   * Tests the connection with the current values from the dialog.
   */
  void TestConnection() const;

  /**
   * Show Advance Authentication UI group based on visible paramter
   *
   * @param visible Boolean indicating visibility
   */
  void ShowAdvanceAuth(bool visble) const;

  /**
   * Show Advanced Options UI group based on visible paramter
   *
   * @param visible Boolean indicating visibility
   */
  void ShowAdvancedOptions(bool visble) const;

  /**
   * Show Log Settings UI group based on visible paramter
   *
   * @param visible Boolean indicating visibility
   */
  void ShowLogSettings(bool visble) const;

  /**
   * Show tab window when tab selection changes
   *
   * @param idx Selected tab window index
   */
  void OnSelChanged(TabIndex::Type idx);

  /**
   * Disable / Enable the fields in advance authentication options UI group
   * based on value of AuthType.
   * AuthType field is always enabled.
   */
  void OnAuthTypeChanged() const;

  /**
   * Disable / Enable the fields in logging options UI group
   * based on value of LogLevel.
   * LogLevel field is always enabled.
   */
  void OnLogLevelChanged() const;

  /**
   * Create basic settings group box.
   * Basic settings include DSN name, and
   * region and endpoint fields.
   *
   * @param posX X position.
   * @param posY Y position.
   * @param sizeX Width.
   * @return Size by Y.
   */
  int CreateBasicSettingsGroup(int posX, int posY, int sizeX);

  /**
   * Create authentication options group box.
   *
   * @param posX X position.
   * @param posY Y position.
   * @param sizeX Width.
   * @return Size by Y.
   */
  int CreateAuthenticationSettingsGroup(int posX, int posY, int sizeX);

  /**
   * Create Advanced Options group box.
   *
   * @param posX X position.
   * @param posY Y position.
   * @param sizeX Width.
   * @return Size by Y.
   */
  int CreateAdvancedOptionsGroup(int posX, int posY, int sizeX);

  /**
   * Create logging configuration settings group box.
   *
   * @param posX X position.
   * @param posY Y position.
   * @param sizeX Width.
   * @return Size by Y.
   */
  int CreateLogSettingsGroup(int posX, int posY, int sizeX);

  /**
   * Retrieves current values from the children and stores
   * them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveParameters(config::Configuration& cfg) const;

  /**
   * Retrieves current values from the basic settings UI group and
   * stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveBasicParameters(config::Configuration& cfg) const;

  /**
   * Retrieves current values from the basic authentication UI group and
   * stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveBasicAuthParameters(config::Configuration& cfg) const;

  /**
   * Retrieves current values from the advance authentication options UI group
   * and stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveAdvanceAuthParameters(config::Configuration& cfg) const;

  /**
   * Retrieves current values from the Connection UI group and
   * stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveConnectionParameters(config::Configuration& cfg) const;

  /**
   * Retrieves current values from the log configuration UI group and
   * stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveLogParameters(config::Configuration& cfg) const;

  /** Window width. */
  int width;

  /** Window height. */
  int height;

  /** DSN name edit field. */
  std::unique_ptr< Window > nameEdit;

  /** DSN name field label. */
  std::unique_ptr< Window > nameLabel;

  /** DSN name balloon. */
  std::unique_ptr< EDITBALLOONTIP > nameBalloon;

  /** Endpoint edit. */
  std::unique_ptr< Window > endpointEdit;

  /** Endpoint label. */
  std::unique_ptr< Window > endpointLabel;

  /** Region edit. */
  std::unique_ptr< Window > regionEdit;

  /** Region label. */
  std::unique_ptr< Window > regionLabel;

  /** Tabs. */
  std::unique_ptr< Window > tabs;

  /** TabsGroupBox. */
  std::unique_ptr< Window > tabsGroupBox;

  /** Auth Type comboBox **/
  std::unique_ptr< Window > authTypeComboBox;

  /** Auth Type label. */
  std::unique_ptr< Window > authTypeLabel;

  /** Access key Id edit. */
  std::unique_ptr< Window > accessKeyIdEdit;

  /** Access key Id label. */
  std::unique_ptr< Window > accessKeyIdLabel;

  /** Secret Access Key edit. */
  std::unique_ptr< Window > secretAccessKeyEdit;

  /** Secret Access Key label. */
  std::unique_ptr< Window > secretAccessKeyLabel;

  /** Session Token edit. */
  std::unique_ptr< Window > sessionTokenEdit;

  /** Session Token label. */
  std::unique_ptr< Window > sessionTokenLabel;

  /** Profile Name edit. */
  std::unique_ptr< Window > profileNameEdit;

  /** Profile Name label. */
  std::unique_ptr< Window > profileNameLabel;

  /** Role ARN edit. */
  std::unique_ptr< Window > roleArnEdit;

  /** Role ARN label. */
  std::unique_ptr< Window > roleArnLabel;

  /** IdP User Name edit. */
  std::unique_ptr< Window > idPUserNameEdit;

  /** IdP User Name label. */
  std::unique_ptr< Window > idPUserNameLabel;

  /** IdP Password edit. */
  std::unique_ptr< Window > idPPasswordEdit;

  /** IdP Password label. */
  std::unique_ptr< Window > idPPasswordLabel;

  /** IdP ARN edit. */
  std::unique_ptr< Window > idPArnEdit;

  /** IdP ARN label. */
  std::unique_ptr< Window > idPArnLabel;

  /** IdP Host edit. */
  std::unique_ptr< Window > idPHostEdit;

  /** IdP Host label. */
  std::unique_ptr< Window > idPHostLabel;

/*$*/
  /** Okta Application Id edit. */
  std::unique_ptr< Window > oktaAppIdEdit;

  /** Okta Application Id label. */
  std::unique_ptr< Window > oktaAppIdLabel;

  /** AAD Application Id edit. */
  std::unique_ptr< Window > aadAppIdEdit;

  /** AAD Application Id label. */
  std::unique_ptr< Window > aadAppIdLabel;

  /** AAD Client Secret edit. */
  std::unique_ptr< Window > aadClientSecretEdit;

  /** AAD Client Secret label. */
  std::unique_ptr< Window > aadClientSecretLabel;

  /** AAD Tenant edit. */
  std::unique_ptr< Window > aadTenantEdit;

  /** AAD Tenant label. */
  std::unique_ptr< Window > aadTenantLabel;

  /** Connection timeout edit. */
  std::unique_ptr< Window > connectionTimeoutEdit;

  /** Connection timeout field label. */
  std::unique_ptr< Window > connectionTimeoutLabel;

  /** Request timeout edit. */
  std::unique_ptr< Window > reqTimeoutEdit;

  /** Request timeout field label. */
  std::unique_ptr< Window > reqTimeoutLabel;

  /** Max retry count client edit. */
  std::unique_ptr< Window > maxRetryCountClientEdit;

  /** Max retry count client field label. */
  std::unique_ptr< Window > maxRetryCountClientLabel;

  /** Max connections edit. */
  std::unique_ptr< Window > maxConnectionsEdit;

  /** Max connections field label. */
  std::unique_ptr< Window > maxConnectionsLabel;

  /** Max connections balloon. */
  std::unique_ptr< EDITBALLOONTIP > maxConnectionsBalloon;

  /** Log Level comboBox **/
  std::unique_ptr< Window > logLevelComboBox;

  /** Log Level label. */
  std::unique_ptr< Window > logLevelLabel;

  /** Log Path edit. */
  std::unique_ptr< Window > logPathEdit;

  /** Log Path label. */
  std::unique_ptr< Window > logPathLabel;

  /** Browse button. */
  std::unique_ptr< Window > browseButton;

  /** Test button. */
  std::unique_ptr< Window > testButton;

  /** Ok button. */
  std::unique_ptr< Window > okButton;

  /** Cancel button. */
  std::unique_ptr< Window > cancelButton;

  /* Driver Version Label */
  std::unique_ptr< Window > versionLabel;

  /** Configuration. */
  config::Configuration& config;

  /** Flag indicating whether OK option was selected. */
  bool accepted;

  /** Flag indicating whether the configuration window has been created. */
  bool created;

  /** Flag indicating whether the DSN name balloon has been shown. */
  bool shownNameBalloon;

  /** Flag indicating whether the max connections balloon has been shown. */
  bool shownMaxConBalloon;

  /** The previously selected tab index.  */
  TabIndex::Type preSel;
};
}  // namespace ui
}  // namespace system
}  // namespace odbc
}  // namespace timestream

#endif  //_TIMESTREAM_ODBC_SYSTEM_UI_DSN_CONFIGURATION_WINDOW
