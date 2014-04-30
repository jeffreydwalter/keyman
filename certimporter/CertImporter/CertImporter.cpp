// CertImporter.cpp :
// This application imports DER encoded X509 certificates into the current
// user's personal trust store.
//
// certimporter <path_to_certificate> <system store name (e.g. ROOT)>
//
// If and only if the import was successful, this program exits with status 0.

#include "stdafx.h"
#include <windows.h>
#include <Wincrypt.h>
#include <iostream>
#include <fstream>

using namespace std;

const int STR_BUF_SZ = 4096;
const int STATUS_BAD_PARAMS = 99;

// See http://msdn.microsoft.com/en-us/library/windows/desktop/aa382363(v=vs.85).aspx
int checkExists(HCERTSTORE store, LPCWSTR expectedName) {
	PCCERT_CONTEXT cert = CertFindCertificateInStore(
		store,
		X509_ASN_ENCODING,
		0,                
		CERT_FIND_SUBJECT_STR,
		expectedName,
		NULL);
	if (cert) {
		return 0;
	}
	cerr << "No certificate was found with common name " << expectedName;
	return 2;
}

// See http://www.idrix.fr/Root/Samples/capi_pem.cpp
// See http://msdn.microsoft.com/en-us/library/windows/desktop/aa382037(v=vs.85).aspx
// See http://blogs.msdn.com/b/alejacma/archive/2008/01/31/how-to-import-a-certificate-without-user-interaction-c-c.aspx
int addCert(HCERTSTORE store, LPCWSTR certFileName) {
	// Open the certificate file
	ifstream certFile;
	certFile.open(certFileName, ios::in | ios::binary | ios::ate);
	if (!certFile.is_open()) {
		cerr << "Unable to open cert file: " << certFileName << endl;
		return 2;
	}

	// Read the certificate file into memory
	// Note - tellg gives us the size because we opened the file with ios::ate, which puts
	// the cursor at the end of the file.
	streampos size = certFile.tellg();
	char *memblock = new char[size];
	// Now jump back to the beginning of the file and read it into memory
	certFile.seekg(0, ios::beg);
	certFile.read(memblock, size);
	certFile.close();

	// Parse the certificate
	PCCERT_CONTEXT cert = CertCreateCertificateContext(
		X509_ASN_ENCODING,
		(BYTE *)memblock,
		size);
	if (cert == NULL) {
		cerr << "Unable to create CertCreateCertificateContext: " << GetLastError() << " data: " << memblock << endl;
		return 3;
	}

	if (CertAddCertificateContextToStore(
		store,
		cert,
		CERT_STORE_ADD_REPLACE_EXISTING,
		NULL
		) == FALSE) {
		cerr << "CertAddCertificateContextToStore error: " << GetLastError() << endl;
		return 4;
	}

	return 0;
}
// See http://www.idrix.fr/Root/Samples/capi_pem.cpp for the basis of this
int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	// Parse arguments
	if (argc < 4) {
		cerr << "Not enough arguments" << endl;
		return STATUS_BAD_PARAMS;
	}

	LPCWSTR action = argv[1];
	LPCWSTR storeName = argv[2];
	LPCWSTR actionData = argv[3];

	// Figure out which action to take
	int(*actionFn)(HCERTSTORE, LPCWSTR);
	if (wcsncmp(action, L"find", 4) == 0) {
		actionFn = checkExists;
	} else if (wcsncmp(action, L"add", 3) == 0) {
		actionFn = addCert;
	} else {
		cerr << "Invalid action: " << action << endl;
		return STATUS_BAD_PARAMS;
	}

	// Open the system store into which to add the certificate
	// See https://groups.google.com/forum/#!topic/microsoft.public.dotnet.security/iIkP0mkf5f4
	HCERTSTORE store = CertOpenStore(
		CERT_STORE_PROV_SYSTEM,
		X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		0,
		CERT_SYSTEM_STORE_LOCAL_MACHINE,
		storeName);
	if (store == NULL) {
		cerr << "Unable to open " << storeName << " cert store: " << GetLastError() << endl;
		return 1;
	}

	return actionFn(store, actionData);
}