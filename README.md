## Tarball Extract
This is a lightweight way to decompress and extract files from a tarball in C.  
Tarball here means a gzipped tar archive of the form filename.tar.gz. It uses  
zlib for gzip decompression and microtar to extract files from the archive. It  
is meant as a reference for how to combine these two libraries in C.  

#### Building
Windows only, but the only platform specific code is creating a directory. Should be easy to port.  
On windows, make sure you have access to the msvc compiler in a command prompt, then from the project root run:  
`build`  

#### Example
To extract the included example run:  
`build\tarball_extract.exe test-dir.tar.gz`  
