# BAR - Backup ARchiver

An archiver with multithreading capabilities which uses remote differential compression to create differential and incremental backups.

## Commands
* a  - archive
* d  - delete
* e  - extract
* ee - extract from archive folder (+ incrementals)
* l  - list
* s  - signature
* t  - test
* u  - update
* uu - folder update
* z  - separate


## Archiving
```BAR a archive.bar -r -s -x *\.git\* -x *.lib  c:\folder1```

Options to "a" command:
* -r				: Recurse subdirectories
* -s				: Put a signature to the archive in order to use later with incremental backup
* --threads [num]	: Number of threads. If not used, maximum OS cores are used.
* -o				: Overwrite archive if it exists. If this is not specified, an existing archive is updated
* -t				: Test run, do not actually create the archive
* -i [mask]			: Add these files. By default, all files are added except those excluded by -x
* -x [mask]			: Exclude these files
* --ir, --xr		: Include/Exclude regex
* -p [pwd]			: Password. This can be used in any "a" , "e" , "t", "u" or "uu". The password is hashed with SHA-256 and AES-256 is used to encrypt the contents and the signature.
* -y                : Yes to all questions


## Generating signature
```BAR s archive.bar```

You can use this command if the archive was not created with the -s option earlier.

## List
```BAR l archive.bar```

Options to "l" command:
* -i [mask]			: Extract these files. By default, all files are extract except those excluded by -x
* -x [mask]			: Exclude these files
* --ir, --xr		: Include/Exclude regex

## Separate
```BAR z archive.bar archive2.bar
   BAR z https://www.example.com/1.bar local.bar
```

Creates an archive that only contains signatures. This is enough for incremental updates to work. Z command works also with URLs.


## Creating an update
```BAR u archive2.bar --incr archive.bar -r -s -x *\.git\* -x *.lib  c:\folder1```

Now archive2.bar contains an incremental backup from archive.bar

```BAR u archive3.bar --incr archive.bar --incr archive2.bar -r -s -x *\.git\* -x *.lib  c:\folder1```

Now archive3.bar contains an incremental backup from archive2.bar which contains an incremental backup from archive1.bar

Options to "u" command are the same as "a" except -o. You can use also a plain signature file created with the "z" command.


## Auto backup
Use the uu option with all the parameters "a" supports, with a target as directory:

```bar uu o:\BACKUP\1 -r -s -x *\.git\* -x *.rar -x *.dll -x *.exe c:\somefolder```

This command will scan the target directory for backup files. If not found, a full backup will be generated.
Next time the command is issued will generate an incremental backup on the same folder.

## Auto backup to Google/One drive
It can use my RGF library (https://github.com/WindowsNT/rgf) to automatically backup to onedrive/google (and soon, dropbox).

```bar uu google:fold\backups\1 --rgf r.xml -r -s -x *\.git\* -x *.rar -x *.dll -x *.exe c:\somefolder```

The --rgf parameter specifies a XML file with the following format:

```
<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>
<e>
	<tokens>
		<google u="clientid" p="clientsecret" t1="accesstoken" t2="refreshtoken"/>
		<onedrive u="clientid" p="clientsecret" t1="accesstoken" t2="refreshtoken"/>
	</tokens>
</e>
```

If you do not have an access token, BAR will use the RGF tools to request one from the server (listening by default to port 9932 on localhost)



## Extract
```BAR e archive.bar c:\out```

```BAR e archive2.bar c:\out```

```BAR e archive3.bar c:\out```

Options to "e" command:
* --threads [num]	: Number of threads. If not used, maximum OS cores are used.
* -t				: Test run, do not actually extract
* -i [mask]			: Extract these files. By default, all files are extract except those excluded by -x
* -x [mask]			: Exclude these files
* --ir, --xr		: Include/Exclude regex

You can also use "ee" with a folder. This way all the archives (and their incremental backups) are extracted.


## Delete

```BAR d archive.bar *.txt *.docx```

You can also use -i and -x to delete to include/exclude items to be deleted, for example 
```BAR d archive.bar *.txt -x f11.txt``


