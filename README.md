# xml_loader
Tcp-server:
- read all xml-files from directory and write it to SQLite database;
- process reply from clients (request contents table rows).

Tcp-client (GUI-application):
- automatically try connect to server;
- apply to change server connection settings (IP and port);
- apply to send a request and view reply content as a tree.

Thus, we can see a little example of "data migration": from xml (which is a tree-based structure) to DB-tables, from DB-tables to simple network protocol and from network protocol to tree-view representation.
