# Torrent-File-Sharing-System
This Application let users to upload or download a file from a group they belong to.
File is Downloaded chunkwise, using piecewise selection algorithm and chunks of a file is downloaded parallel
from multiple servers.

# Authentication
  - Registration is needed for a new user, and login is required for old user to enter into Applicaion.
  
# Functions Implemented
- Create Group
  - Any user can create his own group to share his files with other users.
- upload Files
  - After creating a group, ownner can upload files that he wanted to share, users that belongs to the group can also upload files that they wanted to share.
- Join Group
  - Any user can join any group in order to get access of files belonging to that group.To join a group user has to send 
  request to tracker server, and this request is forwarded to group owner.
- List Request
  - It gives all requests recieved to join a group. Only group owner can view the list of requests recieved.
- Accept Request
  - It lets group owner to accept request and add new user to his group. Once request is accepted newuser has all the rights to upload/download files in/from that particular group.
    only Group owner can accept recieved requests.
- List Groups
  - Newuser can view list of all groups present at current instance to know the groupid he wants to join.
- List Files
  - any user can view list of files shared by a particular group, In this way it's easy to join that group, only where that particular file is present.
- Download Files
  - Any user that belongs to a group can download any file of that particular group. If a user doesnot belong to a group than he needs to first join that group than only he can access files in that group.
- Trackers
  - Central Tracker enables connected peers to download and share files with each other, also includes tracker synchronisation for reliability enhancement.
    It also maintains Hash value of each file so that downloaded file hash value canbe matched with original hash to ensure correctness of a file.
    
# Concepts and Technologies
  - c++ programing Language
  - Multithreading in c++
  - File Handling
  - Socket Programming
  - Ubuntu 18.04 OS
  
#### note:- Application runs on localhost machine
