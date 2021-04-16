# Net_Binary
Built a programs that mimics some capabilities of net.exe

Net.exe is a executable written C++ that attempts to mimic some capabilities of the actual NET.EXE tool. This programming was written with the uses of several Windows32 API to help aid in its functionality. 

## Startup

***Operating System**

This tool was built and tested only on a Windows 10 operating system. As such, the performance of the tool on older versions of Windows is not known at this time. 

## Usage

This tool mimics just a few of NET.EXE's capabilities. The commands that can be run with this tool are:

	Net.exe [ group | localgroup | share | use | user | view ]

Keep in mind that your system configurations and user privilege's may also affect this tool's outputs, some commands may not executes as they require Admin and Remote accesses. In addition, these six commands currently only support a limited number of options and are very specific on how they should be entered. Please go over the syntax rules below:


*Please note - All arguments in the commands found below must contain a value. If you don't need a particular option, enter **"NONE"** for that argument in the order found below*


#### Net.exe group

	 Net.exe group [<GroupName> <UserName>] {/add | /delete}

#### Net.exe localgroup

	 Net.exe localgroup [<GroupName> <Name>] {/add | /delete}

#### Net.exe share

	 Net.exe <ShareName> [<Drive>:<DirectoryPath] [/delete]

#### Net.exe use

	 Net.exe <DeviceName> [\\<ComputerName>\<ShareName>] [/delete]

#### Net.exe user

	 Net.exe[<UserName> <Password>] [/add | /delete] [/domain]

#### Net.exe view

	 Net.exe [\\ComputerName] [/cache] [/all]

