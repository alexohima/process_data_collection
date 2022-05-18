# process_data_collection
Implement a program that will launch a specified process and periodically (with a provided time interval) collect the following data about it:
- CPU usage (percent);
-	Memory consumption: Working Set and Private Bytes (for Windows systems) or Resident Set Size and Virtual Memory Size (for Linux systems);
-	Number of open handles (for Windows systems) or file descriptors (for Linux systems).
# Build
CMD command:
g++ main.cpp -o r.exe
# Run
CMD command:
```
r.exe "C:\Program Files\VideoLAN\VLC\vlc.exe" 2
```

- the first argument is the location of the executable file
- the second argument is the time interval of the data collection(in seconds)


```
this code will create a log file named log.csv in the current folder
```
