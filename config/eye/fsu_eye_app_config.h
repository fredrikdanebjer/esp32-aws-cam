/*
* @file fsu_eye_app_config.c
*
* The MIT License (MIT)
*
* Copyright (c) 2021 Fredrik Danebjer
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

#ifndef FSU_EYE_APP_CONFIG__H
#define FSU_EYE_APP_CONFIG__H

/*
 * @brief Frequency at which to upload image data to AWS
 */
#define FSU_EYE_IMAGE_REPORT_FREQ_SECONDS            "2"

/*
 * @brief Frequency at which to upload diagnostic information to AWS
 */
#define FSU_EYE_INFO_REPORT_FREQ_SECONDS             "900" // Every 15 minutes

#endif /* FSU_EYE_APP_CONFIG__H */
