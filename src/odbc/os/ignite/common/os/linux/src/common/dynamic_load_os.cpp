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
 */

#include <dlfcn.h>

#include <sstream>
#include "ignite/common/dynamic_load_os.h"
#include "trino/odbc/utility.h"

namespace ignite {
namespace odbc {
namespace common {
namespace dynamic {
Module::Module() : handle(NULL) {
  // No-op.
}

Module::Module(void* handle) : handle(handle) {
  // No-op.
}

Module::~Module() {
  // No-op.
}

Module::Module(const Module& other) : handle(other.handle) {
  // No-op.
}

Module& Module::operator=(const Module& other) {
  handle = other.handle;

  return *this;
}

void* Module::FindSymbol(const char* name) {
  return dlsym(handle, name);
}

bool Module::IsLoaded() const {
  return handle != NULL;
}

void Module::Unload() {
  if (IsLoaded())
    dlclose(handle);
}

Module LoadModule(const wchar_t* path) {
  std::string path0 = trino::odbc::utility::ToUtf8(path);
  void* handle = dlopen(path0.c_str(), RTLD_NOW);

  return Module(handle);
}

Module LoadModule(const std::wstring& path) {
  return LoadModule(path.c_str());
}

Module GetCurrent() {
  return LoadModule(NULL);
}
}  // namespace dynamic
}  // namespace common
}  // namespace odbc
}  // namespace ignite
