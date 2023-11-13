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

#include "trino/odbc/system/ui/window.h"
#include "trino/odbc/ignite_error.h"

#include <windowsx.h>
#include <CommCtrl.h>

namespace trino {
namespace odbc {
namespace system {
namespace ui {
HINSTANCE GetHInstance() {
  HINSTANCE hInstance =
      GetModuleHandle(utility::FromUtf8(TARGET_MODULE_FULL_NAME).c_str());

  if (hInstance == NULL) {
    std::stringstream buf;

    buf << "Can not get hInstance for the module, error code: "
        << GetLastError();

    throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
  }

  return hInstance;
}

Window::Window(Window* parent, const std::wstring& className,
               const std::wstring& title)
    : className(className),
      title(title),
      handle(NULL),
      created(false),
      parent(parent) {
  // No-op.
}

Window::Window(HWND handle)
    : className(), title(), handle(handle), created(false), parent(nullptr) {
  // No-op.
}

Window::~Window() {
  if (created)
    Destroy();
}

void Window::Create(DWORD style, int posX, int posY, int width, int height,
                    int id) {
  if (handle) {
    std::stringstream buf;

    buf << "Window already created, error code: " << GetLastError();

    throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
  }

  handle = CreateWindow(className.c_str(), title.c_str(), style, posX, posY,
                        width, height, parent ? parent->GetHandle() : nullptr,
                        reinterpret_cast< HMENU >(static_cast< ptrdiff_t >(id)),
                        GetHInstance(), this);

  if (!handle) {
    std::stringstream buf;

    buf << "Can not create window, error code: " << GetLastError();

    throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
  }

  created = true;

  HGDIOBJ hfDefault = GetStockObject(DEFAULT_GUI_FONT);

  SendMessage(GetHandle(), WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
}

void Window::Show() {
  ShowWindow(handle, SW_SHOW);
}

void Window::Update() {
  UpdateWindow(handle);
}

void Window::Destroy() {
  if (handle)
    DestroyWindow(handle);

  handle = nullptr;
}

void Window::GetText(std::wstring& text) const {
  if (!IsEnabled()) {
    text.clear();

    return;
  }

  int len = GetWindowTextLength(handle);

  if (len <= 0) {
    text.clear();

    return;
  }

  text.resize(len + 1);

  if (!GetWindowText(handle, &text[0], len + 1))
    text.clear();

  text.resize(len);
}

void Window::SetText(const std::wstring& text) const {
  SNDMSG(handle, WM_SETTEXT, 0, reinterpret_cast< LPARAM >(text.c_str()));
}

bool Window::HasText() const {
  return IsEnabled() && GetWindowTextLength(handle) > 0;
}

bool Window::IsChecked() const {
  return IsEnabled() && Button_GetCheck(handle) == BST_CHECKED;
}

void Window::SetChecked(bool state) {
  Button_SetCheck(handle, state ? BST_CHECKED : BST_UNCHECKED);
}

void Window::AddString(const std::wstring& str) {
  SNDMSG(handle, CB_ADDSTRING, 0, reinterpret_cast< LPARAM >(str.c_str()));
}

void Window::SetCBSelection(int idx) {
  SNDMSG(handle, CB_SETCURSEL, static_cast< WPARAM >(idx), 0);
}

int Window::GetCBSelection() const {
  return static_cast< int >(SNDMSG(handle, CB_GETCURSEL, 0, 0));
}

void Window::SetTabSelection(int idx) {
  SNDMSG(handle, TCM_SETCURSEL, static_cast< WPARAM >(idx), 0);
}

int Window::GetTabSelection() const {
  return static_cast< int >(SNDMSG(handle, TCM_GETCURSEL, 0, 0));
}

void Window::SetEnabled(bool enabled) {
  EnableWindow(GetHandle(), enabled);
}

bool Window::IsEnabled() const {
  return IsWindowEnabled(GetHandle()) != 0;
}

void Window::AddTab(int idx, wchar_t* tabTitle) {
  TCITEM tabItem;
  tabItem.mask = TCIF_TEXT;
  tabItem.iImage = -1;
  tabItem.pszText = tabTitle;

  if (SNDMSG(handle, TCM_INSERTITEM, static_cast< WPARAM >(idx),
             reinterpret_cast< LPARAM >(&tabItem))
      == -1) {
    DestroyWindow(handle);

    std::stringstream buf;

    buf << "Can not add new tab, error code: " << GetLastError();

    throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
  }
}
}  // namespace ui
}  // namespace system
}  // namespace odbc
}  // namespace trino
