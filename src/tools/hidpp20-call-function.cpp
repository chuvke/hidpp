/*
 * Copyright 2015 Clément Vuchener
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <cstdio>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "/dev/hidrawX feature_index function [parameters...]";
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;

	std::vector<Option> options = {
		DeviceIndexOption (device_index),
		VerboseOption (),
	};
	Option help = HelpOption (argv[0], args, &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	if (argc-first_arg < 3) {
		fprintf (stderr, "Too few arguments.\n");
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	char *endptr;

	const char *path = argv[first_arg];
	int feature_index = strtol (argv[first_arg+1], &endptr, 0);
	if (*endptr != '\0' || feature_index < 0 || feature_index > 255) {
		fprintf (stderr, "Invalid feature index.\n");
		return EXIT_FAILURE;
	}
	int function = strtol (argv[first_arg+2], &endptr, 0);
	if (*endptr != '\0' || function < 0 || function > 15) {
		fprintf (stderr, "Invalid function.\n");
		return EXIT_FAILURE;
	}

	std::vector<uint8_t> params, results;
	for (int i = 0; first_arg+3+i < argc; ++i) {
		int value = strtol (argv[first_arg+3+i], &endptr, 16);
		if (*endptr != '\0' || value < 0 || value > 255) {
			fprintf (stderr, "Invalid parameter %d value.\n", i);
			return EXIT_FAILURE;
		}
		params[i] = static_cast<uint8_t> (value);
	}

	try {
		HIDPP20::Device dev (path, device_index);
		results = dev.callFunction (static_cast<uint8_t> (feature_index),
					    static_cast<unsigned int> (function),
					    params);
	}
	catch (HIDPP20::Error e) {
		fprintf (stderr, "Error code %d: %s\n", e.errorCode (), e.what ());
		return e.errorCode ();
	}

	bool first = true;
	for (uint8_t value: results) {
		if (first)
			first = false;
		else
			printf (" ");
		printf ("%02hhx", value);
	}
	printf ("\n");

	return EXIT_SUCCESS;
}
