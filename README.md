# xml_loader
Tcp-server:
- read all xml-files from directory and write it to SQLite database;
- process clients request and send a reply, which contents all rows extracted from database tables.

Tcp-client (GUI-application):
- automatically try connect to server;
- allows to change server connection settings (IP and port);
- allows to send a request and display reply contents as a tree.

Thus, we can see a "data migration" example: from xml (which is a tree-based structure) to DB-tables, from DB-tables to simple network protocol and from network protocol to tree-view representation.
