// +build !darwin,!windows,!linux

package keyman

import (
	"fmt"
)

func DeleteTrustedRootByName(commonName string) error {
	return fmt.Errorf("DeleteTrustedRootByName is not supported on this platform")
}

// AddAsTrustedRoot adds the certificate to the user's trust store as a trusted
// root CA.
func (cert *Certificate) AddAsTrustedRoot() error {
	return fmt.Errorf("AddToUserTrustStore is not supported on this platform")
}

func (cert *Certificate) IsInstalled() (bool, error) {
	return false, fmt.Errorf("IsInstalled is not supported on this platform")
}
