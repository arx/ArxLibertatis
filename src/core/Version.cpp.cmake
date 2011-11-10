/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/Version.h"

/*!
 * This file is automatically processed by cmake if the version or commit id changes.
 * Available variables:
 * - BASE_VERSION: The contents of the VERSION file.
 * - GIT_COMMIT: The current git commit. This variable is not defined if there is no .git directory.
 * - GIT_COMMIT_PREFIX_i: The first i characters of the git commit (i=0..39).
 * For the exact syntax see the documentation of the configure_file() cmake command.
 */

#cmakedefine GIT_COMMIT

#ifdef GIT_COMMIT
const std::string version = "${BASE_VERSION} + ${GIT_COMMIT_PREFIX_5}";
#else
const std::string version = "${BASE_VERSION}";
#endif
