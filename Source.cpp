#include <Windows.h>
#include <string>
#include <stdio.h>
#include <lm.h>
#include <cstring>
#include <wchar.h>
#include <Winnetwk.h>
#include <iostream>

#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "netapi32.lib")


int wmain(int argc, wchar_t *argv[])
{
	// Argument to grab the specific Net commands from the user
	std::wstring argv1 = argv[1];			
	
	// Prompt for the user to inform them of the required syntax
	if (argv[1] == L"/?") {
		std::cout << "\nThe syntax of this command is:\n\nNET\n\t[ group | localgroup | share | use | user | view ] \n" << std::endl;
		std::cout << "Type a Net.exe command followed by the option /? for the syntax of the command\n" << std::endl;
		std::cout << "For Example:\tNet.exe /?\n" << std::endl;
	}

	// Net USE command
	else if (argv1 == L"use") {
		
		// Declaring and intializing variables that store the arguments used in the Net use command
		std::wstring del = argv[4];
		std::wstring compName = argv[3];
		std::wstring drive = argv[2];

		// Net.exe use /?
		if (argv[2] == L"/?") {
			std::cout << "The syntax of this command is:\n\n\tNet.exe use [<DeviceName>] [\\<ComputerName>\<ShareName>] [/delete]" << std::endl;
			std::cout << "*NOTE* - All 3 arguments following \"Net.exe use\" must be \nfulfilled with their corresponding values or";
			std::cout << "with the value \"NONE\"\nPlease refer to the READ.ME file if you need more assistance!\n" << std::endl;
		}

		// "Net.exe <DeviceName> \\<ComputerName>\<ShareName> /delete". Should cancel the specified network connection.
		if ((drive != L"NONE" || compName != L"NONE") && del == L"/delete") {
			std::cout << "Deleting Connection...\n";
			DWORD dwRetVal;
			LPCSTR lpName;
			DWORD dwFlags;
			BOOL fForce = FALSE;		// False = will fail if connection is in use

			// Connection can be canceld by entering either the <DeviceName> or the \\<ComputerName>\<ShareName>
			if (drive == L"NONE")
				lpName = (LPCSTR)argv[3];
			else
				lpName = (LPCSTR)argv[2];
			dwFlags = CONNECT_UPDATE_PROFILE;						// Update changes that are made
			dwRetVal = WNetCancelConnection2A((LPCSTR)lpName,		//Win32 API call
				dwFlags,
				fForce);

			// Error Handling
			if (dwRetVal == NO_ERROR)
				wprintf(L"Connection from %s deleted.\n", lpName);
			else
				wprintf(L"WNetAddConnection2 failed. Error Code: %ld\n", dwRetVal);
		}

		// "Net.exe <DeviceName> \\<ComputerName>\<ShareName> NONE aka "Net.exe use <DeviceName> \\<ComputerName>\<ShareName>"
		// Should assign the device name to the name of the server and the shared resource. 
		if (drive != L"NONE" && compName != L"NONE" && del != L"/delete") {
			std::cout << "Adding Connection...\n";
			DWORD dwRetVal;
			NETRESOURCE nr;
			DWORD dwFlags;

			memset(&nr, 0, sizeof(NETRESOURCE));
			nr.dwType = RESOURCETYPE_ANY;
			nr.lpLocalName = argv[2];					// The Device Name
			nr.lpRemoteName = argv[3];					// The shared resource
			nr.lpProvider = NULL;

			dwFlags = CONNECT_UPDATE_PROFILE;			// Update any changes that were made

			// Win32 API call
			dwRetVal = WNetAddConnection2(&nr, argv[4], argv[3], dwFlags);

			// Error handling
			if (dwRetVal == NO_ERROR)
				wprintf(L"Connection added to %s\n", nr.lpRemoteName);
			else
				wprintf(L"WNetAddConnection2 failed. Error Code: %ld\n", dwRetVal);
		}

		// "Net.exe use NONE NONE NONE" aka "Net.exe use". Should retrieve a list of network connections.  
		if (drive == L"NONE" && compName == L"NONE" && del == L"NONE") {
			LPNETRESOURCE lpnr = NULL;	// Ptr for NETRESOUCRE sturct
			LPNETRESOURCE pTmp = NULL;	// Ptr for NETRESOUCRE sturct
			DWORD dwResult;
			DWORD dwResultEnum;
			HANDLE hEnum;
			DWORD cbBuffer = 16384;     // Set buffer size
			DWORD cEntries = -1;        // Enumerate all possible entries
			LPNETRESOURCE lpnrLocal;    // Pointer to enumerated structures
			DWORD i;

			// Function starts enumeration 
			dwResult = WNetOpenEnum(RESOURCE_GLOBALNET,		 // All connections resources
				RESOURCETYPE_ANY,							 // All resources types
				0,											 // Value 0 = enumerate all resources
				lpnr,
				&hEnum);									 // Handle to the resource

			// Error reporting for WNetOpenEnum function
			if (dwResult == ERROR_NO_NETWORK)
				std::cout << "Network is unavailable." << std::endl;
			else
				wprintf(L"WNetOpenEnum failed. Error Code: %ld\n", dwResult);

			lpnrLocal = (LPNETRESOURCE)GlobalAlloc(GPTR, cbBuffer);			// Allocate resources
			do {
				ZeroMemory(lpnrLocal, cbBuffer);							// Initialize the buffer

				// Function  continues enumeration of network resources 
				// that was started by  WNetOpenEnum function
				dwResultEnum = WNetEnumResource(hEnum,						// Resource handle
					&cEntries,
					lpnrLocal,												// LPNETRESOURCE
					&cbBuffer);												// Buffer size

				if (dwResultEnum == NO_ERROR) {											// WNetEnumResource ran successfully
					for (i = 0; i < cEntries; i++) {									// Loop to go through results
						LPNETRESOURCE pTmp = &lpnrLocal[i];
						if (pTmp->dwDisplayType == RESOURCEDISPLAYTYPE_NETWORK) {		// Checking for only Network connections results
							wprintf(L"DisplayType: Network Connections\n");
							wprintf(L" Scope: ");

							// Swtich to detemine the scope of the enumeration results
							switch (pTmp->dwScope) {
							case (RESOURCE_CONNECTED):
								wprintf(L"connected\n");
								break;
							case (RESOURCE_GLOBALNET):
								wprintf(L"all resources\n");
								break;
							case (RESOURCE_REMEMBERED):
								wprintf(L"remembered\n");
								break;
							default:
								wprintf(L"unknown scope %d\n", pTmp->dwScope);
								break;
							}

							// Display all other relevant info about the network connections
							wprintf(L"NETRESOURCE[%d] Localname: %s\n", i, pTmp->lpLocalName);
							wprintf(L"NETRESOURCE[%d] Remotename: %s\n", i, pTmp->lpRemoteName);
							wprintf(L"NETRESOURCE[%d] Comment: %s\n", i, pTmp->lpComment);
							wprintf(L"NETRESOURCE[%d] Provider: %s\n", i, pTmp->lpProvider);
							wprintf(L"\n");
						}
					}
				}
			} while (dwResultEnum != ERROR_NO_MORE_ITEMS);			// End loop 
			GlobalFree((HGLOBAL)lpnrLocal);							// Free memory that was allocated
			dwResult = WNetCloseEnum(hEnum);						// Stop enumerating
		}
	}

	// Net USER command
	else if (argv1 == L"user") {
		
		// Declaring and intializing variables that store the arguments used in the Net user command
		std::wstring domain = argv[5];
		std::wstring operation = argv[4];
		std::wstring pass = argv[3];
		std::wstring usname = argv[2];

		// Net.exe user /?
		if (argv[2] == L"/?") {
			std::cout << "The syntax of this command is:\n\n\tNet.exe user [<UserName> {<Password>}] [/add | /delete] [/domain]" << std::endl;
			std::cout << "*NOTE* - All 4 arguments following \"Net.exe user\" must be \nfulfilled with their corresponding values or";
			std::cout << "with the value \"NONE\"\nPlease refer to the READ.ME file if you need more assistance!\n" << std::endl;
		}

		// "Net.exe user <UserName> <Password> /add /domain". Should add User to Domain Controller (if available)
		if (usname != L"NONE" && operation == L"/add" && domain == L"/domain") {
			NET_API_STATUS dStatus;
			LPCWSTR lpServer = NULL;
			LPCWSTR lpDomain = NULL;
			LPCWSTR lpDcName = NULL;

			//Get the domain controller
			dStatus = NetGetDCName(lpServer, lpDomain, (LPBYTE*)&lpDcName);
			if (dStatus == NERR_Success) {
				wprintf(L"NetGetDCName was successful\n", dStatus);
				wprintf(L"DC Name = %ws\n", lpDcName);
				dStatus = NetApiBufferFree((LPVOID)lpDcName);
			}
			
			//Error Handling
			if (dStatus != NERR_Success)
				wprintf(L"NetGetDCName failed. Error Code: %ld\n", dStatus);


			USER_INFO_1 ui;
			DWORD dwLevel = 1;					// Level 1 Struct
			DWORD dwError = 0;
			NET_API_STATUS nStatus;

			//Populate the User Struct
			ui.usri1_name = argv[2];
			ui.usri1_password = argv[3];
			ui.usri1_priv = USER_PRIV_USER;
			ui.usri1_home_dir = NULL;
			ui.usri1_comment = NULL;
			ui.usri1_flags = UF_SCRIPT;
			ui.usri1_script_path = NULL;

			// Win32 API call to Add user
			nStatus = NetUserAdd(lpDcName,
				dwLevel,
				(LPBYTE)&ui,
				&dwError);

			// Error Handling
			if (nStatus == NERR_Success)
				fwprintf(stderr, L"User %s has been successfully added on %s\n", argv[2], argv[1]);
			else
				fprintf(stderr, "A system error has occurred. Error Code: %ld\n", nStatus);
		}
		
		// "Net.exe user <UserName> NONE /delete /domain". Should delete User from Domain Controller (if available)
		if (usname != L"NONE" && pass == L"NONE" && operation == L"/delete" && domain == L"/domain") {
			NET_API_STATUS dStatus;
			LPCWSTR lpServer = NULL;
			LPCWSTR lpDomain = NULL;
			LPCWSTR lpDcName = NULL;

			// Get the domain controller
			dStatus = NetGetDCName(lpServer, lpDomain, (LPBYTE*)&lpDcName);
			if (dStatus == NERR_Success) {
				wprintf(L"NetGetDCName was successful\n", dStatus);
				wprintf(L"DC Name = %ws\n", lpDcName);
				dStatus = NetApiBufferFree((LPVOID)lpDcName);
			}
			if (dStatus != NERR_Success)
				wprintf(L"NetGetDCName failed. Error Code: %ld\n", dStatus);

			DWORD dwError = 0;
			NET_API_STATUS nStatus;

			// Win32 API call to Delete user
			nStatus = NetUserDel(lpDcName, argv[2]);

			// Error Handling
			if (nStatus == NERR_Success)
				fwprintf(stderr, L"User %s has been successfully deleted from %s\n", argv[2], lpDcName);
			else
				fprintf(stderr, "A system error has occurred. Error Code: %ld\n", nStatus);
		}
		
		// "Net.exe user <UserName> <Password> /add NONE". Should add User to Local Machine
		if (usname != L"NONE" && pass != L"NONE" && operation == L"/add" && domain == L"NONE") {
			USER_INFO_1 ui;
			DWORD dwLevel = 1;
			DWORD dwError = 0;
			NET_API_STATUS nStatus;

			// populating the USER Struct
			ui.usri1_name = argv[2];
			ui.usri1_password = argv[3];
			ui.usri1_priv = USER_PRIV_USER;
			ui.usri1_home_dir = NULL;
			ui.usri1_comment = NULL;
			ui.usri1_flags = UF_SCRIPT;
			ui.usri1_script_path = NULL;

			// Win32 API call to Add user
			nStatus = NetUserAdd(NULL,
				dwLevel,
				(LPBYTE)&ui,
				&dwError);

			// Error Handling
			if (nStatus == NERR_Success)
				fwprintf(stderr, L"User %s has been successfully added on %s\n", argv[2], argv[1]);
			else
				fprintf(stderr, "A system error has occurred. Error Code: %ld\n", nStatus);
		}
		
		// "Net.exe user <UserName> NONE /delete NONE". Should delete User from Local Machine
		if (usname != L"NONE" && pass == L"NONE" && operation == L"/delete" && domain == L"NONE") {
			DWORD dwError = 0;
			NET_API_STATUS nStatus;

			// Win32 API call to Delete User
			nStatus = NetUserDel(NULL, argv[2]);

			// Error Handling
			if (nStatus == NERR_Success)
				fwprintf(stderr, L"User %s has been successfully deleted from Local Machine\n", argv[2]);
			else
				fprintf(stderr, "A system error has occurred. Error Code: %ld\n", nStatus);
		}
		
		// "Net.exe user <UserName> NONE NONE NONE" aka "Net.exe user <UserName>". Should display information about the user
		if (usname != L"NONE" && pass == L"NONE" && operation == L"NONE" && domain == L"NONE") {
			LPUSER_INFO_11 pBuf = NULL;
			LPUSER_INFO_11 pBuf11 = NULL;
			DWORD dwLevel = 11;
			int j;
			NET_API_STATUS nStatus;
			LPTSTR pszServerName = NULL;

			pszServerName = (LPTSTR)argv[2];			// set username from cli
			do {
				nStatus = NetUserGetInfo(NULL, (LPCWSTR)pszServerName, dwLevel, (LPBYTE*)&pBuf);		// Win32 API to get user info
				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					if (pBuf != NULL) {
						pBuf11 = (LPUSER_INFO_11)pBuf;													//using Ptr to help print info
						wprintf(L"\tUser account name: %s\n", pBuf11->usri11_name);
						wprintf(L"\tComment: %s\n", pBuf11->usri11_comment);
						wprintf(L"\tUser comment: %s\n", pBuf11->usri11_usr_comment);
						wprintf(L"\tFull name: %s\n", pBuf11->usri11_full_name);
						wprintf(L"\tPrivilege level: %d\n", pBuf11->usri11_priv);
						wprintf(L"\tAuth flags (in hex): %x\n", pBuf11->usri11_auth_flags);
						wprintf(L"\tPassword age (seconds): %d\n", pBuf11->usri11_password_age);
						wprintf(L"\tHome directory: %s\n", pBuf11->usri11_home_dir);
						wprintf(L"\tParameters: %s\n", pBuf11->usri11_parms);
						wprintf(L"\tLast logon (seconds since January 1, 1970 GMT): %d\n", pBuf11->usri11_last_logon);
						wprintf(L"\tLast logoff (seconds since January 1, 1970 GMT): %d\n", pBuf11->usri11_last_logoff);
						wprintf(L"\tBad password count: %d\n", pBuf11->usri11_bad_pw_count);
						wprintf(L"\tNumber of logons: %d\n", pBuf11->usri11_num_logons);
						wprintf(L"\tLogon server: %s\n", pBuf11->usri11_logon_server);
						wprintf(L"\tCountry code: %d\n", pBuf11->usri11_country_code);
						wprintf(L"\tWorkstations: %s\n", pBuf11->usri11_workstations);
						wprintf(L"\tMax storage: %d\n", pBuf11->usri11_max_storage);
						wprintf(L"\tUnits per week: %d\n", pBuf11->usri11_units_per_week);
						wprintf(L"\tLogon hours:");
						for (j = 0; j < 21; j++) {
							printf(" %x", (BYTE)pBuf11->usri11_logon_hours[j]);
						}
						wprintf(L"\n");
						wprintf(L"\tCode page: %d\n", pBuf11->usri11_code_page);
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"User information for %s was successfully displayed.\n", argv[2]);
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", nStatus);


			// Get Local group info for the user
			LPLOCALGROUP_USERS_INFO_0 lgpBuf = NULL;
			DWORD lgdwLevel = 0;
			DWORD lgdwFlags = LG_INCLUDE_INDIRECT;
			DWORD lgdwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD lgdwEntriesRead = 0;
			DWORD lgdwTotalEntries = 0;
			NET_API_STATUS lgnStatus;

			lgnStatus = NetUserGetLocalGroups(NULL,			// Win32 API call
				(LPCWSTR)argv[2],
				lgdwLevel,
				lgdwFlags,
				(LPBYTE*)&lgpBuf,
				lgdwPrefMaxLen,
				&lgdwEntriesRead,
				&lgdwTotalEntries);

			if (lgnStatus == NERR_Success) {
				LPLOCALGROUP_USERS_INFO_0 lgpTmpBuf;
				DWORD lgi;
				DWORD lgdwTotalCount = 0;

				if ((lgpTmpBuf = lgpBuf) != NULL) {
					wprintf(L"\tLocal Groups Membership: ");			// display local groups
					for (lgi = 0; lgi < lgdwEntriesRead; lgi++) {
						wprintf(L"%s\n", lgpTmpBuf->lgrui0_name);
						lgpTmpBuf++;
						lgdwTotalCount++;
					}
				}
			}

			// Error Handling
			if (lgnStatus == NERR_Success)
				wprintf(L"Localgroups for %s was successfully displayed.\n", argv[2]);
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", lgnStatus);

			// Get the global group for th user
			LPGROUP_USERS_INFO_0 gpBuf = NULL;
			DWORD gdwLevel = 0;
			DWORD gdwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD gdwEntriesRead = 0;
			DWORD gdwTotalEntries = 0;
			NET_API_STATUS gnStatus;

			gnStatus = NetUserGetGroups(NULL,							// Win32 API call
				(LPCWSTR)argv[2],
				gdwLevel,
				(LPBYTE*)&gpBuf,
				gdwPrefMaxLen,
				&gdwEntriesRead,
				&gdwTotalEntries);

			if (gnStatus == NERR_Success) {
				LPGROUP_USERS_INFO_0 gpTmpBuf;
				DWORD gi;
				DWORD gdwTotalCount = 0;

				if ((gpTmpBuf = gpBuf) != NULL) {
					wprintf(L"\tGlobal Group Memberships: ");			// Display the groups
					for (gi = 0; gi < gdwEntriesRead; gi++) {
						wprintf(L"%s\n", gpTmpBuf->grui0_name);
						gpTmpBuf++;
						gdwTotalCount++;
					}
				}
			}

			// Error Handling
			if (gnStatus == NERR_Success)
				wprintf(L"Global Groups for %s was successfully displayed.\n", argv[2]);
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", gnStatus);
		}
		
		// "Net.exe user NONE NONE NONE NONE" aka "Net.exe user". Should display a list of all users for the Local Machine
		if (usname == L"NONE" && pass == L"NONE" && operation == L"NONE" && domain == L"NONE"){
			LPUSER_INFO_0 pBuf = NULL;
			LPUSER_INFO_0 pTmpBuf;
			DWORD dwLevel = 0;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwResumeHandle = 0;
			DWORD i;
			DWORD dwTotalCount = 0;
			NET_API_STATUS nStatus;
			LPTSTR pszServerName = NULL;

			do {
				nStatus = NetUserEnum((LPCWSTR)pszServerName,			// Win32 API call to enumerate the Users on the Local Machine
					dwLevel,
					FILTER_NORMAL_ACCOUNT,								// global users
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);

				// Win32 API is successful
				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					if ((pTmpBuf = pBuf) != NULL) {
						for (i = 0; (i < dwEntriesRead); i++) {									// Looping through the results 
							wprintf(L"\t-- %s\n", pTmpBuf->usri0_name);							// Reading usernames from the USER struct
							pTmpBuf++;
							dwTotalCount++;
						}
					}
				}
			}
			while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Successfully enumerated all users on the Local Machine.\n");
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", nStatus);
		}
		
		// "Net.exe user <UserName> NONE NONE /domain" aka "Net.exe user <UserName> /domain". Should display information about the user
		if (usname != L"NONE" && pass == L"NONE" && operation == L"NONE" && domain == L"/domain") {
			NET_API_STATUS dStatus;
			LPCWSTR lpServer = NULL;
			LPCWSTR lpDomain = NULL;
			LPCWSTR lpDcName = NULL;

			// Get local Domain
			dStatus = NetGetDCName(lpServer, lpDomain, (LPBYTE*)&lpDcName);
			if (dStatus == NERR_Success) {
				wprintf(L"NetGetDCName was successful\n", dStatus);
				wprintf(L"DC Name = %ws\n", lpDcName);
				dStatus = NetApiBufferFree((LPVOID)lpDcName);
			}
			if (dStatus != NERR_Success)
				wprintf(L"NetGetDCName failed. Error Code: %ld\n", dStatus);

			// Get User information
			LPUSER_INFO_11 pBuf = NULL;
			LPUSER_INFO_11 pBuf11 = NULL;
			DWORD dwLevel = 11;
			int i;
			int j;
			NET_API_STATUS nStatus;
			LPTSTR pszServerName = NULL;

			pszServerName = (LPTSTR)argv[2];
			do {
				nStatus = NetUserGetInfo(NULL, (LPCWSTR)pszServerName, dwLevel, (LPBYTE*)&pBuf);
				std::cout << nStatus << "\n";
				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					wprintf(L"Command Ran Successfully for User Justin\n");
					if (pBuf != NULL) {
						pBuf11 = (LPUSER_INFO_11)pBuf;
						wprintf(L"\tUser account name: %s\n", pBuf11->usri11_name);
						wprintf(L"\tComment: %s\n", pBuf11->usri11_comment);
						wprintf(L"\tUser comment: %s\n", pBuf11->usri11_usr_comment);
						wprintf(L"\tFull name: %s\n", pBuf11->usri11_full_name);
						wprintf(L"\tPrivilege level: %d\n", pBuf11->usri11_priv);
						wprintf(L"\tAuth flags (in hex): %x\n", pBuf11->usri11_auth_flags);
						wprintf(L"\tPassword age (seconds): %d\n", pBuf11->usri11_password_age);
						wprintf(L"\tHome directory: %s\n", pBuf11->usri11_home_dir);
						wprintf(L"\tParameters: %s\n", pBuf11->usri11_parms);
						wprintf(L"\tLast logon (seconds since January 1, 1970 GMT): %d\n", pBuf11->usri11_last_logon);
						wprintf(L"\tLast logoff (seconds since January 1, 1970 GMT): %d\n", pBuf11->usri11_last_logoff);
						wprintf(L"\tBad password count: %d\n", pBuf11->usri11_bad_pw_count);
						wprintf(L"\tNumber of logons: %d\n", pBuf11->usri11_num_logons);
						wprintf(L"\tLogon server: %s\n", pBuf11->usri11_logon_server);
						wprintf(L"\tCountry code: %d\n", pBuf11->usri11_country_code);
						wprintf(L"\tWorkstations: %s\n", pBuf11->usri11_workstations);
						wprintf(L"\tMax storage: %d\n", pBuf11->usri11_max_storage);
						wprintf(L"\tUnits per week: %d\n", pBuf11->usri11_units_per_week);
						wprintf(L"\tLogon hours:");
						for (j = 0; j < 21; j++) {
							printf(" %x", (BYTE)pBuf11->usri11_logon_hours[j]);
						}
						wprintf(L"\n");
						wprintf(L"\tCode page: %d\n", pBuf11->usri11_code_page);
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			LPLOCALGROUP_USERS_INFO_0 lgpBuf = NULL;			// Get localgroups
			DWORD lgdwLevel = 0;
			DWORD lgdwFlags = LG_INCLUDE_INDIRECT;
			DWORD lgdwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD lgdwEntriesRead = 0;
			DWORD lgdwTotalEntries = 0;
			NET_API_STATUS lgnStatus;

			lgnStatus = NetUserGetLocalGroups(NULL,
				(LPCWSTR)pszServerName,
				lgdwLevel,
				lgdwFlags,
				(LPBYTE*)&lgpBuf,
				lgdwPrefMaxLen,
				&lgdwEntriesRead,
				&lgdwTotalEntries);

			if (lgnStatus == NERR_Success) {
				LPLOCALGROUP_USERS_INFO_0 lgpTmpBuf;
				DWORD lgi;
				DWORD lgdwTotalCount = 0;

				if ((lgpTmpBuf = lgpBuf) != NULL) {
					wprintf(L"\tLocal Groups Membership: ");
					for (lgi = 0; lgi < lgdwEntriesRead; lgi++) {
						if (lgpTmpBuf == NULL) {
							fprintf(stderr, "An access violation has occurred\n");
							break;
						}
						wprintf(L"%s\n", lgpTmpBuf->lgrui0_name);

						lgpTmpBuf++;
						lgdwTotalCount++;
					}
				}
			}

			// Error Handling
			if (lgnStatus == NERR_Success)
				wprintf(L"Local Groups for %s was successfully displayed.\n", argv[2]);
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", lgnStatus);

			LPGROUP_USERS_INFO_0 gpBuf = NULL;
			DWORD gdwLevel = 0;
			DWORD gdwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD gdwEntriesRead = 0;
			DWORD gdwTotalEntries = 0;
			NET_API_STATUS gnStatus;

			gnStatus = NetUserGetGroups(NULL,						// Get Global Groups
				(LPCWSTR)pszServerName,
				gdwLevel,
				(LPBYTE*)&gpBuf,
				gdwPrefMaxLen,
				&gdwEntriesRead,
				&gdwTotalEntries);

			if (gnStatus == NERR_Success) {
				LPGROUP_USERS_INFO_0 gpTmpBuf;
				DWORD gi;
				DWORD gdwTotalCount = 0;

				if ((gpTmpBuf = gpBuf) != NULL) {
					wprintf(L"\tGlobal Group Memberships: ");
					for (gi = 0; gi < gdwEntriesRead; gi++) {
						if (gpTmpBuf == NULL) {
							fprintf(stderr, "An access violation has occurred\n");
							break;
						}
						wprintf(L"%s\n", gpTmpBuf->grui0_name);

						gpTmpBuf++;
						gdwTotalCount++;
					}
				}
			}

			// Error Handling
			if (gnStatus == NERR_Success)
				wprintf(L"Global Groups for %s was successfully displayed.\n", argv[2]);
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", gnStatus);
		}
		
		// "Net.exe user NONE NONE NONE /domain" aka "Net.exe user /domain". Should display users for Domain Controller
		if (usname == L"NONE" && pass == L"NONE" && operation == L"NONE" && domain == L"/domain") {
			LPUSER_INFO_0 pBuf = NULL;
			LPUSER_INFO_0 pTmpBuf;
			DWORD dwLevel = 0;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwResumeHandle = 0;
			DWORD i;
			DWORD dwTotalCount = 0;
			NET_API_STATUS nStatus;
			LPTSTR pszServerName = NULL;

			pszServerName = NULL;
			do {
				nStatus = NetUserEnum((LPCWSTR)pszServerName,		// enumerating users
					dwLevel,
					FILTER_TEMP_DUPLICATE_ACCOUNT,					// users on a Domain Controller
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);
				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					if ((pTmpBuf = pBuf) != NULL) {
						for (i = 0; (i < dwEntriesRead); i++) {
							wprintf(L"\t-- %s\n", pTmpBuf->usri0_name);			//display the usernames
							pTmpBuf++;
							dwTotalCount++;
						}
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Successfully enumerated all users on the Domain Controller.\n");
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", nStatus);
		}
	}

	// Net LOCALGROUP command
	else if (argv1 == L"localgroup") {
		
		// Declaring and intializing variables that store the arguments used in the Net localgroup command
		std::wstring operation = argv[4];
		std::wstring usname = argv[3];
		std::wstring lgpname = argv[2];
		
		// Net.exe localgroup /?
		if (argv[2] == L"/?") {
			std::cout << "The syntax of this command is:\n\n\tNet.exe localgroup [<GroupName> <Name>] [/add | /delete]" << std::endl;
			std::cout << "*NOTE* - All 3 arguments following \"Net.exe localgroup\" must be \nfulfilled with their corresponding values or";
			std::cout << "with the value \"NONE\"\nPlease refer to the READ.ME file if you need more assistance!\n" << std::endl;
		}

		// "Net.exe localgroup <GroupName> NONE /add" aka "Net.exe localgroup <GroupName> /add". Should create a local group named <GroupName>
		if (lgpname != L"NONE" && usname == L"NONE" && operation == L"/add") {
			LPWSTR lpszLocalGroup = argv[2];
			LOCALGROUP_INFO_0 localgroup_info;
			NET_API_STATUS nStatus = 0;
			DWORD parm_err = 0;

			localgroup_info.lgrpi0_name = lpszLocalGroup;				// Write name of local grou in Struct

			nStatus = NetLocalGroupAdd(NULL,							// Win32 API call to create the local group	
				0,														// level 
				(LPBYTE)&localgroup_info,								// input buffer 
				&parm_err);

			if (nStatus == NERR_Success)
				wprintf(L"Local group, %s, successfully created.\n", argv[2]);
			else if (nStatus == ERROR_ALIAS_EXISTS)
				wprintf(L"Local group, %s, already exists.\n", argv[2]);
			else {
				wprintf(L"Error adding local group: %s\n", argv[2]);
				wprintf(L"Error Code: %ld", nStatus);
			}
		}

		// "Net.exe localhost <GroupName> NONE /delete" aka "Net.exe <GroupName> /delete". Shoudl delete a local group named <GroupName>
		if (lgpname != L"NONE" && usname == L"NONE" && operation == L"/delete") {
			NET_API_STATUS nStatus = 0;

			// Win32 API call to delete
			nStatus = NetLocalGroupDel(NULL,	//servername
				(LPCWSTR)argv[2]);				//groupname

			if (nStatus == NERR_Success)
				wprintf(L"Local group, %s, successfully deleted.\n", argv[2]);
			else if (nStatus == NERR_GroupNotFound)
				wprintf(L"Local group, %s, does NOT exists.\n", argv[2]);
			else
				wprintf(L"Error deleting local group: %s\n", argv[2]);
		}

		// "Net.exe localgroup <GroupName> NONE NONE" aka "Net.exe localgroup <GroupName>". Should displays users in the <GroupName> local group
		if (lgpname != L"NONE" && usname == L"NONE" && operation == L"NONE") {
			LPLOCALGROUP_MEMBERS_INFO_1 pBuf = NULL;
			LPLOCALGROUP_MEMBERS_INFO_1 pTmpBuf;
			DWORD dwLevel = 1;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwResumeHandle = 0;
			DWORD i;
			DWORD dwTotalCount = 0;
			NET_API_STATUS nStatus;
			LPCWSTR serverName = NULL;
			LPCWSTR targetGroup = NULL;
			targetGroup = (LPCWSTR) argv[2];								// set group from cli
			do {
				nStatus = NetLocalGroupGetMembers((LPCWSTR)serverName,		// Win32 API call get the members of the local group
					(LPCWSTR)targetGroup,
					dwLevel,
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);
				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					if ((pTmpBuf = pBuf) != NULL) {
						wprintf(L"The Members in the Localgroup, %s, are: \n", argv[2]);		// display memebers of the local group
						for (i = 0; (i < dwEntriesRead); i++) {
							wprintf(L"\t-- %s\n", pTmpBuf->lgrmi1_name);
							pTmpBuf++;
						}
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Users in Local Group, %s, was successfully displayed.\n", argv[2]);
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", nStatus);
		}
		
		// "Net.exe localgroup NONE NONE NONE" aka "Net.exe localgroup". Should display all local groups on the local server.
		if (lgpname == L"NONE" && usname == L"NONE" && operation == L"NONE") {
			LPLOCALGROUP_INFO_0 pBuf = NULL;
			LPLOCALGROUP_INFO_0 pTmpBuf;
			DWORD dwLevel = 0;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwResumeHandle = 0;
			DWORD i;
			DWORD dwTotalCount = 0;
			NET_API_STATUS nStatus;
			LPCWSTR serverName = NULL;

			// Win32 API call to enumerate the local groups
			do {
				nStatus = NetLocalGroupEnum((LPCWSTR)serverName,
					dwLevel,
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);
				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					if ((pTmpBuf = pBuf) != NULL) {
						wprintf(L"The Localgroups on this machine are:\n");				// display the lock groups
						for (i = 0; (i < dwEntriesRead); i++) {
							wprintf(L"\t-- %s\n", pTmpBuf->lgrpi0_name);
							pTmpBuf++;
						}
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Local Groups for Local Server was successfully displayed.\n");
			else
				wprintf(L"A system error has occurred. Error Code: %ld\n", nStatus);
		}
		
		/*
		if (lgpname != L"NONE" && operation == L"/add") {
			LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
			NET_API_STATUS nStatus = 0;
			LPWSTR lpszUser;

			localgroup_members.lgrmi0_sid = ;

			nStatus = NetLocalGroupAddMembers(NULL,        // local computer 
				(LPCWSTR)argv[2],               // group name 
				3,                            // name 
				(LPBYTE)&localgroup_members, // buffer 
				1);                          // count

			if (nStatus == NERR_Success)
				printf("User, %s, successfully added to Localgroup.\n", argv[2]);
			else if (nStatus == ERROR_ALIAS_EXISTS)
				printf("Local group, %s, already exists.\n", argv[2]);
			else
				printf("Error adding local group: %s\n", argv[2]);
				
		}
		if (lgpname != L"NONE" && operation == L"/delete") {

		}
		*/
	}

	// Net GROUP command
	else if (argv1 == L"group") {
		
		// Declaring and intializing variables that store the arguments used in the Net group command
		std::wstring operation = argv[4];
		std::wstring usname = argv[3];
		std::wstring gpname = argv[2];
		
		// Net.exe localgroup /?
		if (argv[2] == L"/?") {
			std::cout << "The syntax of this command is:\n\n\tNet.exe group [<GroupName> <UserName>] [/add | /delete]" << std::endl;
			std::cout << "*NOTE* - All 3 arguments following \"Net.exe group\" must be \nfulfilled with their corresponding values or";
			std::cout << "with the value \"NONE\"\nPlease refer to the READ.ME file if you need more assistance!\n" << std::endl;
		}

		// "Net.exe group <GroupName> NONE /add" aka "Net.exe group <GroupName> /add". Should create a group
		if (gpname != L"NONE" && usname == L"NONE" && operation == L"/add") {
			LPWSTR gpname = argv[2];
			GROUP_INFO_0 group_info;
			NET_API_STATUS nStatus = 0;
			DWORD parm_err = 0;

			group_info.grpi0_name = gpname;

			// Win32 API call add group
			nStatus = NetGroupAdd(NULL,			//servername
				0,								// level 
				(LPBYTE)&group_info,			// input buffer 
				&parm_err);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Group, %s, successfully created.\n", argv[2]);
			else if (nStatus == NERR_GroupExists)
				wprintf(L"Group, %s, already exists.\n", argv[2]);
			else {
				wprintf(L"Error adding group: %s\n", argv[2]);
				wprintf(L"Error Code: %ld\n", nStatus);
			}
		}
		
		// "Net.exe group <GroupName> NONE /delete" aka "Net.exe group <GroupName> /delete". Should delete agroup
		if (gpname != L"NONE" && usname == L"NONE" && operation == L"/delete") {
			NET_API_STATUS nStatus = 0;

			// Win32 API call edlete group
			nStatus = NetGroupDel(NULL,			//servername
				(LPCWSTR)argv[2]);				//groupname

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Group, %s, successfully deleted.\n", argv[2]);
			else if (nStatus == NERR_GroupNotFound)
				wprintf(L"Group, %s, does NOT exists.\n", argv[2]);
			else {
				wprintf(L"Error deleting group: %s\n", argv[2]);
				wprintf(L"Error Code: %ld\n", nStatus);
			}
		}
		
		// "Net.exe group <GroupName> <UserName> /add". Should add a user to a group
		if (gpname != L"NONE" && usname != L"NONE" && operation == L"/add") {
			NET_API_STATUS nStatus = 0;

			// Win32 API add a user to group
			nStatus = NetGroupAddUser(NULL,		// servername
				(LPCWSTR)argv[2],				// groupname
				(LPCWSTR)argv[3]);				// username

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"User, %s, successfully added to the %s group.\n", argv[3], argv[2]);
			else if (nStatus == NERR_GroupNotFound)
				wprintf(L"Group, %s, name not found.\n", argv[2]);
			else {
				wprintf(L"Error adding user, %s, to the %s group.\n", argv[3], argv[2]);
				wprintf(L"Error Code: %ld\n", nStatus);
			}
		}

		// "Net.exe group <GroupName> <UserName> /delete". Should remove a user from a group
		if (gpname != L"NONE" && usname != L"NONE" && operation == L"/delete") {
			NET_API_STATUS nStatus = 0;

			// Win32 API call remove user from group
			nStatus = NetGroupDelUser(NULL,		// servername
				(LPCWSTR)argv[2],				// groupname
				(LPCWSTR)argv[3]);				// username

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"User, %s, successfully removed from the %s group.\n", argv[3], argv[2]);
			else if (nStatus == NERR_GroupNotFound)
				wprintf(L"Group, %s, name not found.\n", argv[2]);
			else if (nStatus == NERR_UserNotFound)
				wprintf(L"Username, %s, not found.\n", argv[3]);
			else {
				wprintf(L"Error adding user, %s, to the %s group.\n", argv[3], argv[2]);
				wprintf(L"Error Code: %ld", nStatus);
			}
		}
		
		// "Net.exe group <GroupName> NONE NONE" aka "Net.exe group <GroupName>". Should display the users in group <GroupName>
		if (gpname != L"NONE" && usname == L"NONE" && operation == L"NONE") {
			LPGROUP_USERS_INFO_0 pBuf = NULL;
			LPGROUP_USERS_INFO_0 pTmpBuf;
			DWORD dwLevel = 0;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwResumeHandle = 0;
			DWORD i;
			DWORD dwTotalCount = 0;
			NET_API_STATUS nStatus;
			LPCWSTR serverName = NULL;
			LPCWSTR targetGroup = NULL;
			targetGroup = (LPCWSTR)argv[2];
			do {
				nStatus = NetGroupGetUsers((LPCWSTR)serverName,
					(LPCWSTR)targetGroup,
					dwLevel,
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);
				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					if ((pTmpBuf = pBuf) != NULL)
					{
						wprintf(L"The Members in the Global Group, %s, are: \n", argv[2]);
						for (i = 0; (i < dwEntriesRead); i++)
						{
							wprintf(L"\t-- %s\n", pTmpBuf->grui0_name);
							pTmpBuf++;
						}
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Successfully displayed the users in the %s Group.\n", argv[2]);
			else
				wprintf(L"Could not display Users from Group %s. Error Code: %ld\n", argv[2], nStatus);
		}

		// "Net.exe group NONE NONE NONE" aka "Net.exe group". Should list all groups on a local server
		if (gpname == L"NONE" && usname == L"NONE" && operation == L"NONE") {
			LPGROUP_INFO_0 pBuf = NULL;
			LPGROUP_INFO_0 pTmpBuf;
			DWORD dwLevel = 0;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwResumeHandle = 0;
			DWORD i;
			DWORD dwTotalCount = 0;
			NET_API_STATUS nStatus;
			LPCWSTR serverName = NULL;

			do {
				nStatus = NetGroupEnum((LPCWSTR)serverName,							//Win32 API to enumerate groups
					dwLevel,
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);
				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					if ((pTmpBuf = pBuf) != NULL) {
						wprintf(L"The Global Group on this machine are:\n");
						for (i = 0; (i < dwEntriesRead); i++) {
							wprintf(L"\t-- %s\n", pTmpBuf->grpi0_name);
							pTmpBuf++;
						}
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Successfully displayed the groups from the local server.\n");
			else
				wprintf(L"Could not display Group. Error Code: %ld\n", nStatus);
		}
	}

	// Net VIEW command
	else if (argv1 == L"view")	{
		
		// Declaring and intializing variables that store the arguments used in the Net view command
		std::wstring all = argv[4];
		std::wstring cache = argv[3];
		std::wstring compName = argv[2];
		
		// Net.exe view /?
		if (argv[2] == L"/?") {
			std::cout << "The syntax of this command is:\n\n\tNet.exe view [\\ComputerName] [/cache] [/all]" << std::endl;
			std::cout << "*NOTE* - All 3 arguments following \"Net.exe view\" must be \nfulfilled with their corresponding values or";
			std::cout << "with the value \"NONE\"\nPlease refer to the READ.ME file if you need more assistance!\n" << std::endl;
		}
		
		// "Net.exe view \\ComputerName NONE /all" aka "Net.exe view \\ComputerName /all". Should display ALL shared resorces 
		if (compName != L"NONE" && all == L"/all") {
			LPSHARE_INFO_502 pBuf = NULL;
			LPSHARE_INFO_502 pTmpBuf;
			DWORD dwLevel = 502;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwResumeHandle = 0;
			DWORD i;
			NET_API_STATUS nStatus;
			LPTSTR serverName = NULL;
			serverName = (LPTSTR)argv[2];

			do {
				nStatus = NetShareEnum((LPTSTR)serverName,		// WIN32 API call display ALL shared resource.
					dwLevel,									// even shows hidden shares!
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);
				if (nStatus == ERROR_SUCCESS || nStatus == ERROR_MORE_DATA)
				{
					if ((pTmpBuf = pBuf) != NULL) {
						wprintf(L"Share:              Local Path:                   Uses:   Remarks:\n");
					}
					for (i = 1; i <= dwEntriesRead; i++) {
						wprintf(L"%-20s%-30s%-8u%s\n", pTmpBuf->shi502_netname, pTmpBuf->shi502_path, pTmpBuf->shi502_current_uses, pTmpBuf->shi502_remark);
						pTmpBuf++;
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Successfully displayed all of %s's Shared resourcse.\n", argv[2]);
			else {
				wprintf(L"Error displaying ALL shares.\n");
				wprintf(L"Error Code: %ld", nStatus);
			}
		}

		// "Net.exe view \\ComputerName NONE NONE" aka "Net.exe view". Should display shared resources of \\ComputerName
		if (compName != L"NONE" && all == L"NONE") {
			LPNETRESOURCE lpnr = NULL;
			LPNETRESOURCE pTmp = NULL;
			DWORD dwResult;
			DWORD dwResultEnum;
			HANDLE hEnum;
			DWORD cbBuffer = 16384;     // Buffer size
			DWORD cEntries = -1;        // enumerate all possible entries
			LPNETRESOURCE lpnrLocal;    // pointer to enumerated structures
			DWORD i;

			dwResult = WNetOpenEnum(RESOURCE_GLOBALNET,		// all connections resources
				RESOURCETYPE_ANY,							// all resources
				0,											// enumerate all resources
				lpnr,										// NULL first time the function is called
				&hEnum);									// handle to the resource
			wprintf(L"Error Code: %s\n", dwResult);
			lpnrLocal = (LPNETRESOURCE)GlobalAlloc(GPTR, cbBuffer);		// allocate
			do {
				ZeroMemory(lpnrLocal, cbBuffer);
				dwResultEnum = WNetEnumResource(hEnum,  // resource handle
					&cEntries,
					lpnrLocal,							// LPNETRESOURCE
					&cbBuffer);							// buffer size
				if (dwResultEnum == NO_ERROR) {

					for (i = 0; i < cEntries; i++) {
						LPNETRESOURCE pTmp = &lpnrLocal[i];
						if (pTmp->dwDisplayType == RESOURCEDISPLAYTYPE_SHARE) {					// Display shared resources
							wprintf(L"NETRESOURCE[%d] DisplayType: ", i);						// will not show hidden shares
							wprintf(L"shares\n");
							wprintf(L"NETRESOURCE[%d] Scope: ", i);
							switch (pTmp->dwScope) {
							case (RESOURCE_CONNECTED):
								wprintf(L"connected\n");
								break;
							case (RESOURCE_GLOBALNET):
								wprintf(L"all resources\n");
								break;
							case (RESOURCE_REMEMBERED):
								wprintf(L"remembered\n");
								break;
							default:
								wprintf(L"unknown scope %d\n", pTmp->dwScope);
								break;
							}
							wprintf(L"NETRESOURCE[%d] Localname: %s\n", i, pTmp->lpLocalName);
							wprintf(L"NETRESOURCE[%d] Remotename: %s\n", i, pTmp->lpRemoteName);
							wprintf(L"NETRESOURCE[%d] Comment: %s\n", i, pTmp->lpComment);
							wprintf(L"NETRESOURCE[%d] Provider: %s\n", i, pTmp->lpProvider);
							wprintf(L"\n");
						}
						else
							wprintf(L"No Available Shares on Computer Named: %s\n", argv[2]);			// Error Handling
					}
				}
			} while (dwResultEnum != ERROR_NO_MORE_ITEMS);
			wprintf(L"Error Code: %s\n", dwResultEnum);						// Error Handling
			GlobalFree((HGLOBAL)lpnrLocal);									// free memory
			dwResult = WNetCloseEnum(hEnum);								// stop enumerating
		}
		
		// "Net.exe view NONE NONE NONE" aka "Net.exe view". Should display a list of computer in curent domain
		if (compName == L"NONE" && all == L"NONE") {
			LPSERVER_INFO_101 pBuf = NULL;
			LPSERVER_INFO_101 pTmpBuf;
			DWORD dwLevel = 101;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwTotalCount = 0;
			DWORD dwServerType = SV_TYPE_SERVER;        // all servers
			DWORD dwResumeHandle = 0;
			NET_API_STATUS nStatus;
			LPWSTR pszServerName = NULL;
			LPWSTR pszDomainName = NULL;
			DWORD i;

			nStatus = NetServerEnum(pszServerName,		//Win32 API for enumberating all servers on domain
				dwLevel,
				(LPBYTE*)&pBuf,
				dwPrefMaxLen,
				&dwEntriesRead,
				&dwTotalEntries,
				dwServerType,
				pszDomainName,
				&dwResumeHandle);

			if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
				if ((pTmpBuf = pBuf) != NULL) {
					for (i = 0; i < dwEntriesRead; i++) {								// Loop to display through the results.
						printf("\tPlatform: %d\n", pTmpBuf->sv101_platform_id);
						wprintf(L"\tName:     %s\n", pTmpBuf->sv101_name);
						printf("\tVersion:  %d.%d\n",
							pTmpBuf->sv101_version_major,
							pTmpBuf->sv101_version_minor);
						printf("\tType:     %d", pTmpBuf->sv101_type);
						wprintf(L"\tComment:  %s\n\n", pTmpBuf->sv101_comment);
					
						pTmpBuf++;
						dwTotalCount++;
					}
					printf("Successfully displayed all servers on local domain.");
				}
				else
					printf("No servers were found\n");									// Error Handling
			}
			else
				wprintf(L"NetServerEnum failed. Error Code: %ld\n", nStatus);			// Error Handling
		}
	}

	// Net SHARE command
	else if (argv1 == L"share") {
		
		// Declaring and intializing variables that store the arguments used in the Net share command
		std::wstring operation = argv[4];
		std::wstring path = argv[3];
		std::wstring shName = argv[2];

		// Net.exe shared /?
		if (argv[2] == L"/?") {
			std::cout << "The syntax of this command is:\n\n\tNet.exe share <ShareName> [<Drive>:<DirectoryPath] [/delete]" << std::endl;
			std::cout << "*NOTE* - All 3 arguments following \"Net.exe shared\" must be \nfulfilled with their corresponding values or";
			std::cout << "with the value \"NONE\"\nPlease refer to the READ.ME file if you need more assistance!\n" << std::endl;
		}

		// "Net.exe share <ShareName> NONE /delete" aka "Net.exe share <ShareName> /delete". Should stop sharing 
		if (shName != L"NONE" && path == L"NONE" && operation == L"/delete") {
			NET_API_STATUS nStatus;

			nStatus = NetShareDel(NULL,			// Server name
				argv[2],						// Share name
				0);								// Reserved, must be 0


			// Error Reporting
			if (nStatus == NERR_Success)
				wprintf(L" Share, %s, successfully removed.\n", argv[2]);
			else if (nStatus == NERR_NetNameNotFound)
				wprintf(L"Share, %s, name not found.\n", argv[2]);
			else {
				wprintf(L"Error removing %s.\n", argv[2]);
				wprintf(L"Error Code: %ld", nStatus);
			}
		}
		
		// "Net.exe share <ShareName> <Drive>:<DirectoryPath> NONE" aka "Net.exe share <ShareName> <Drive>:<DirectoryPath>.
		// Should share <Drive>:<DirectoryPath> with <ShareName>
		if (shName != L"NONE" && path != L"NONE" && operation == L"NONE") {
			NET_API_STATUS nStatus;
			SHARE_INFO_2 p;
			DWORD parm_err = 0;

			p.shi2_netname = argv[2];			// Share Name
			p.shi2_type = STYPE_DISKTREE;		// Disk drive
			p.shi2_permissions = ACCESS_ALL;
			p.shi2_max_uses = 4;				// Number of concurrent connections
			p.shi2_current_uses = 0;
			p.shi2_path = argv[3];
			p.shi2_passwd = NULL;				// No password

			nStatus = NetShareAdd(NULL,			// Servername
				2,								// Struct level
				(LPBYTE)&p,						// Ptr to buf that specifies data
				&parm_err);

			// Error Reporting
			if (nStatus == NERR_Success)
				wprintf(L" Resource, %s, successfully shared to %s path.\n", argv[2], argv[3]);
			else if (nStatus == NERR_GroupNotFound)
				wprintf(L"Group, %s, name not found.\n", argv[2]);
			else if (nStatus == NERR_DuplicateShare)
				wprintf(L"The share name, %s, already exist.\n", argv[2]);
			else {
				wprintf(L"Error sharing, %s, to the %s path.\n", argv[2], argv[3]);
				wprintf(L"Error Code: %ld", nStatus);
			}
		}
		
		// "Net.exe share <ShareName> NONE NONE" aka "Net.exe share <ShareName>". Should display infomration about <ShareName> only
		if (shName != L"NONE" && path == L"NONE" && operation == L"NONE") {
			LPSHARE_INFO_502 BufPtr;
			LPSHARE_INFO_502 pBuf502;
			NET_API_STATUS nStatus;
			LPTSTR serverName = NULL;
			LPTSTR shareName = NULL;
			shareName = (LPTSTR)argv[2];

			do {
				nStatus = NetShareGetInfo((LPTSTR)serverName,			// Win32 API call get share infomation
					(LPTSTR)shareName,
					502,
					(LPBYTE*)&BufPtr);

				if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
					if (BufPtr != NULL) {
						pBuf502 = (LPSHARE_INFO_502)BufPtr;
						wprintf(L"Share Name: %s\n", pBuf502->shi502_netname);
						//wprintf(L"Type: %s\n", pBuf502->shi502_type);
						wprintf(L"Remarks: %s\n", pBuf502->shi502_remark);
						wprintf(L"Permissions: %d\n", pBuf502->shi502_permissions);
						//wprintf(L"Max Uses: %s\n", pBuf502->shi502_max_uses);
						wprintf(L"Current Uses: %u\n", pBuf502->shi502_current_uses);
						wprintf(L"Path: %s\n", pBuf502->shi502_path);
						wprintf(L"Password: %s\n", pBuf502->shi502_passwd);
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Successfully displayed information on the %s Shared resources.\n", argv[2]);
			else
				wprintf(L"Could not display Shared resources. Error Code: %ld\n", nStatus);
		}
		
		// "Net.exe share NONE NONE NONE" aka "Net.exe share". Should display information about shared resources on the local computer
		if (shName == L"NONE" && path == L"NONE" && operation == L"NONE") {
			LPSHARE_INFO_502 pBuf = NULL;
			LPSHARE_INFO_502 pTmpBuf;
			DWORD dwLevel = 502;
			DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
			DWORD dwEntriesRead = 0;
			DWORD dwTotalEntries = 0;
			DWORD dwResumeHandle = 0;
			DWORD i;
			NET_API_STATUS nStatus;
			LPTSTR serverName = NULL;

			do {
				nStatus = NetShareEnum((LPTSTR)serverName,			//Enumerate Shares
					dwLevel,
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);
				if (nStatus == ERROR_SUCCESS || nStatus == ERROR_MORE_DATA) {
					if ((pTmpBuf = pBuf) != NULL) {
						wprintf(L"Share:              Local Path:                   Uses:   Remarks:\n");
					}
					for (i = 1; i <= dwEntriesRead; i++) {
						wprintf(L"%-20s%-30s%-8u%s\n", pTmpBuf->shi502_netname, pTmpBuf->shi502_path, pTmpBuf->shi502_current_uses, pTmpBuf->shi502_remark);
						pTmpBuf++;
					}
				}
			} while (nStatus == ERROR_MORE_DATA);

			// Error Handling
			if (nStatus == NERR_Success)
				wprintf(L"Successfully displayed information on Shared resources.\n");
			else
				wprintf(L"Could not display Shared resources. Error Code: %ld\n", nStatus);
		}
	}

	// Return 0, program ran and exited successfully
	else {
		return 0;
	}
}