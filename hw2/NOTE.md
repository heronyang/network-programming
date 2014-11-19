## Tool Commands
- ipcs: list shared memory/semaphore
- ipcrm: (control) -m/-s

## Features
- remove all the environment variables except PATH
- every client will have its own PATH

## Functions Abstract
- meet with
- work with
- talk to
- make friends

## Commands
(no pipe connected, detect earier)
- who
- tell
- yell
- name

## [tell]
- sender:
    ```
    % tell <client id> <message>
    ```
- reciever:
    ```
    *** <sender name> told you ***: <message>
    ```
- if not exists:
    ```
    *** Error: user #<client id> does not exist yet. ***
    ```

## [yell]
- sender:
    ```
    % yell <message>
    ```
- recievers (all):
    ```
    *** <sender name> yelled ***: <message>
    ```

## [name]
- sender:
    ```
    % name <name>
    ```
- recievers (all):
    ```
    % *** User from <IP>/<PORT> is named '<name>'. ***
    ```
- if exists already:
    ```
    *** User '<name>' already exists. ***
    ```

## [who]
```
   % who
   <ID>	<nickname>	<IP/port>	<indicate me>
   1	IamStudent	140.113.215.62/1201	<-me
   2	(no name)	140.113.215.63/1013
   3	student3	140.113.215.64/1302
```
- client id is assigned when login, retrieved when logout (1~30)

## Events 
### Connect (ok)
```
   % *** User '(no name)' entered from <IP>/<PORT>. ***
```
### Disconnect (ok)
```
   % *** User '<client name>' left. ***
```

## Pipe
stdout, stderr
### Send
    - broadcast:
    ```
        *** <client name> (#<client id>) just piped '<command line>' to <receiver's name> (#<receiver's client_id>) ***
    ```
    - if the user doesn't exist:
    ```
        *** Error: user #<client id> does not exist yet. ***
    ```
    - if sending for the second time (no one read):
    ```
        *** Error: the pipe #2->#1 already exists. ***
    ```
### Recieve
    - broadcast:
    ```
        *** <my name> (#<my client id>) just received from <other client's name> (#<other client's id>) by '<command line>' ***
    ```
    - if the pipe is not exist:
    ```
        *** Error: the pipe #(client id)->#(client id) does not exist yet. ***
    ```

## Notices
- close pipe if disconnect (next user won't get the connect if having same client id)
- broadcast includes the target
- size of client name <= 20

## Requirements
- two different server programs
    - use the single-process concurrent paradigm
    - use the concurrent connection-orient paradigm with shared memory
        - chat buffer >= 10 unread messages (>=1024 bytes for each)
        - ...
