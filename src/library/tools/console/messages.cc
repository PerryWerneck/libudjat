/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <config.h>
#include <udjat/defs.h>
#include <udjat/ui/console.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/string.h>

using namespace std;

namespace Udjat {

	UDJAT_API const Console::Color Console::color(Logger::Level level) {

		// https://alligatr.co.uk/ansi-codes/
		// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

		static const struct {
			Logger::Level level;
			const Console::Color color;
		} decorations[] = {
			{ Logger::Level::Error, 	BrightRedForeground		},
			{ Logger::Level::Notice, 	BrightCyanForeground	},
			{ Logger::Level::Warning,	BrightYellowForeground	},
			{ Logger::Level::Info, 		BrightGreenForeground	},
			{ Logger::Level::Trace, 	BrightBlueForeground	},
			{ Logger::Level::Debug, 	BrightMagentaForeground	},
		};

		for(const auto &decoration : decorations) {
			if(decoration.level & level) {
				return decoration.color;
			}
		}

		return "";

	}

	UDJAT_API const char * Console::icon(Logger::Level level) {

		static const struct {
			Logger::Level level;
			const char *prefix;
		} prefixes[] = {
			{ Logger::Notice, 	"🪧"	},
			{ Logger::Error, 	"❌"		},
			{ Logger::Warning, 	"⚠️"	},
			{ Logger::Info, 	"✅"		},
			{ Logger::Trace, 	"⚙️"	},
			{ Logger::Debug, 	"🪲"	},

		};

		for(const auto &prefix : prefixes) {
			if(level & prefix.level) {
				return prefix.prefix;
			}
		}

		return "🪧";
	}

	UDJAT_API void Console::status(Logger::Level level, const char *domain, const char *message) noexcept {

		if(!Console::decorated()) {
			Logger::String{message}.write(level,domain);
			return;
		}

		bool state = Logger::console();
		Logger::console(false);
		Logger::String{message}.write(level, domain );
		Logger::console(state);

		// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
		Console::write(String{
			"\r",
			ClearEOL,
			color(level),
			icon(level),
			"\t",
			SetBold,
			message,
			Reset,
			"\n"
		}.c_str());


	}

}
