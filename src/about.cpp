/*
 * Copyright (C) 2025, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <iostream>

#include "mnxvalidate.h"

namespace mnxvalidate {
#include "mnxvalidate_license.xxd"
}

namespace nlohmann {
#include "nlohmann_json_license.xxd"
namespace json_schema {
#include "json_schema_validator_license.xxd"

}
}

static const std::basic_string_view<unsigned char> mnxvalidateLicense(mnxvalidate::LICENSE, mnxvalidate::LICENSE_len);
static const std::basic_string_view<unsigned char> nlohmannLicense(nlohmann::LICENSE_MIT, nlohmann::LICENSE_MIT_len);
static const std::basic_string_view<unsigned char> jsonSchemaValidatorLicense(nlohmann::json_schema::LICENSE, nlohmann::json_schema::LICENSE_len);

static const std::array<std::pair<std::string_view, std::basic_string_view<unsigned char>>, 3> licenses = {{
    { "mnxvalidate", mnxvalidateLicense },
    { "nlohmann json", nlohmannLicense },
    { "json schema validator", jsonSchemaValidatorLicense },
}};

namespace mnxvalidate {

void showAboutPage()
{
    for (const auto& next : licenses) {
        std::cout << "=================" << std::endl;
        std::cout << next.first << std::endl;
        std::cout << "=================" << std::endl;
        std::string cstrBuffer(next.second.begin(), next.second.end());
        std::cout << cstrBuffer << std::endl;
    }
}

} // namespace mnxvalidate
