DROP DATABASE IF EXISTS mydiscord;
CREATE DATABASE discord_test;
USE discord_test; /* doesn't work in postgres, use createdb or psql -U postgres -d MyDiscord */

ALTER TABLE Message ADD COLUMN Id_sender INTEGER NOT NULL;
ALTER TABLE Message ADD CONSTRAINT fk_message_sender FOREIGN KEY (Id_sender) REFERENCES Client(Id_client);

CREATE TABLE IF NOT EXISTS Client(
   Id_client SERIAL,
   name VARCHAR(50) NOT NULL,
   last_name VARCHAR(50) NOT NULL,
   pseudo VARCHAR(50) NOT NULL,
   password VARCHAR(150) NOT NULL,
   email VARCHAR(50) NOT NULL,
   PRIMARY KEY(Id_client),
   UNIQUE(pseudo),
   UNIQUE(email)
);

CREATE TABLE IF NOT EXISTS Channel(
   Id_channel SERIAL,
   name_channel VARCHAR(50) NOT NULL,
   is_private BOOLEAN NOT NULL DEFAULT false,
   PRIMARY KEY(Id_channel),
   UNIQUE(name_channel)
);

CREATE TABLE IF NOT EXISTS Message(
   Id_message SERIAL,
   contenu VARCHAR(500),
   date_message TIMESTAMP,
   Id_channel INTEGER NOT NULL,
   Id_sender INTEGER NOT NULL,
   PRIMARY KEY(Id_message),
   FOREIGN KEY(Id_channel) REFERENCES Channel(Id_channel),
   FOREIGN KEY(Id_sender) REFERENCES Client(Id_client)
);

CREATE TYPE emoji as ENUM ('smile', 'sad', 'angry', 'love', 'laugh', 'surprised', 'wink', 'crying', 'thumbsup', 'thumbsdown');
CREATE TABLE IF NOT EXISTS Reaction(
   Id_Reaction SERIAL,
   emoji emoji, /* emoji type = ENUM('smile', 'sad', 'angry', 'love', 'laugh', 'surprised', 'wink', 'crying', 'thumbsup', 'thumbsdown') */
   Id_client INTEGER NOT NULL,
   Id_message INTEGER NOT NULL,
   PRIMARY KEY(Id_Reaction),
   FOREIGN KEY(Id_client) REFERENCES Client(Id_client),
   FOREIGN KEY(Id_message) REFERENCES Message(Id_message)
);

CREATE TABLE IF NOT EXISTS Private_message(
   Id_private_message SERIAL,
   contenu VARCHAR(500),
   date_message TIMESTAMP,
   Id_sender INTEGER NOT NULL,
   Id_target INTEGER NOT NULL,
   PRIMARY KEY(Id_private_message),
   FOREIGN KEY(Id_sender) REFERENCES Client(Id_client),
   FOREIGN KEY(Id_target) REFERENCES Client(Id_client)
);

CREATE TYPE role as ENUM ('admin', 'mod', 'user');
CREATE TABLE IF NOT EXISTS Channel_user(
   Id_client INTEGER,
   Id_channel INTEGER,
   role role, /* role type = ENUM(admin, mod, user)*/
   PRIMARY KEY(Id_client, Id_channel),
   FOREIGN KEY(Id_client) REFERENCES Client(Id_client),
   FOREIGN KEY(Id_channel) REFERENCES Channel(Id_channel)
);
