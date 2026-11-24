# FAT32 File System Utility

[Description]

## Group Members
- **John Doe**: jd19@fsu.edu
- **Jane Smith**: js19@fsu.edu
- **Alex Brown**: ab19@fsu.edu
## Division of Labor

### Part 1: Mounting the Image

- Open the FAT32 image from the first command-line argument. If it cannot be opened, report the
  error and exit. Show the image name and current directory in the shell prompt.
- `info` reads the boot sector and reports the root cluster number, bytes per sector,
  sectors per cluster, data-region cluster count, entries in one FAT, and image size in bytes.
  Label each value and print it on its own line.
- `exit` closes the image and frees the shell's resources.
- **Assigned to**: John Doe

### Part 2: Navigation

- `cd DIRNAME` changes the current directory and updates the prompt. A missing name or a name
  that refers to a file produces an error and leaves the shell in the current directory.
- `ls` lists file and directory names in the current directory, including `.` and `..`.
- **Assigned to**: Jane Smith

### Part 3: Create

- `mkdir DIRNAME` creates a directory in the current directory.
- `creat FILENAME` creates a zero-byte file in the current directory.
- Both commands reject a name already used by either a file or a directory.
- **Assigned to**: Alex Brown

### Part 4: Read

- `open FILENAME FLAGS` adds an existing file to the open-file table with offset zero. Accept
  `-r`, `-w`, `-rw`, or `-wr`. Report a missing file, a duplicate open, or an invalid mode.
- `close FILENAME` removes the file from the table. Report an error if it is missing or closed.
- `lsof` lists the index, name, mode, byte offset, and path of each open file. Report when the
  table is empty. The assignment assumes at most ten files are open at once.
- `lseek FILENAME OFFSET` sets an open file's byte offset. Reject a missing or closed file and
  any offset beyond the end of the file.
- `read FILENAME SIZE` reads up to `SIZE` bytes from the current offset, stops at end of file,
  and advances the offset by the number of bytes read. Require a file opened for reading.
- **Assigned to**: Alex Brown, Jane Smith

### Part 5: Update

- `write FILENAME "STRING"` writes the quoted text at the current offset of a file opened for
  writing. Extend the file as needed and advance the offset by the number of bytes written.
- `mv SOURCE DESTINATION` renames a file or directory, or moves it into an existing destination
  directory. Report an error if the source is missing, the source file is open, or the
  destination is an existing file.
- **Assigned to**: Alex Brown, Jane Smith

### Part 6: Delete

- `rm FILENAME` removes a closed file's directory entry and frees its data clusters. Reject a
  missing name, a directory, or an open file.
- `rmdir DIRNAME` removes an empty directory's entry from its parent and frees its clusters.
  Exclude `.` and `..` from the emptiness check and protect both from removal. Reject a missing
  directory, a file, a nonempty directory, or a directory containing an open file.
- **Assigned to**: Jane Smith

### Extra Credit
- **Responsibilities**: [Description]
- **Assigned to**: Alex Brown

## File Listing
```
filesys/
│
├── src/
│ ├── lexer.c
│ └── ...
│
├── include/
│ └── lexer.h
│ └── ...
│
├── README.md
└── Makefile
```
## How to Compile & Execute

### Requirements
- **Compiler**: e.g., `gcc` for C/C++, `rustc` for Rust.
- **Dependencies**: List any libraries or frameworks necessary (rust only).

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...

## Bugs
- **Bug 1**: This is bug 1.
- **Bug 2**: This is bug 2.
- **Bug 3**: This is bug 3.

## Extra Credit
- **Extra Credit 1**: [Extra Credit Option]
- **Extra Credit 2**: [Extra Credit Option]
- **Extra Credit 3**: [Extra Credit Option]

## Considerations
[Description]
