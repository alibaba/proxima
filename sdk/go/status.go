/**
 *   Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
 * 
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *   
 *       http://www.apache.org/licenses/LICENSE-2.0
 *   
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *   \author   guonix
 *   \date     Mar 2021
 *   \brief
 */

package be

import (
	"encoding/json"
	"errors"
	"fmt"
)

// Status object
type Status struct {
	// Response code from ProximaSE, 0 for success, otherwise for failed
	Code int32 `json:"code"`
	// Detail message explain failed reason, Optional field
	Reason string `json:"reason,omitempty"`
}

// OK check status
func (s *Status) OK() bool {
	return s.Code == 0
}

func (s *Status) IsGrpcError() bool {
	return s.Code > 0
}

// Error msg
func (s *Status) Error() string {
	if s.OK() {
		return ""
	} else if s.IsGrpcError() {
		return fmt.Sprintf("rpc error: code = %d desc = %s", s.Code, s.Reason)
	}
	return fmt.Sprintf(fmt.Sprintf("proxima be error: code = %d desc = %s", s.Code, s.Reason))
}

// String return json formatted string of Status
func (s *Status) String() string {
	if bytes, err := json.Marshal(s); err == nil {
		return string(bytes)
	}
	return "{}"
}

// WrapStatus wrap error as Status, for further actions
func WrapStatus(err error) (*Status, error) {
	if s, ok := err.(*Status); ok {
		return s, nil
	}
	return nil, errors.New("Wrap error as sdk.Status failed.")
}
