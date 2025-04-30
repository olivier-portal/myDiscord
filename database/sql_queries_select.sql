SELECT table_name 
FROM information_schema.tables 
WHERE table_schema = 'public';

select * from client;
SELECT * FROM message;
SELECT * FROM private_message;

SELECT n.nspname AS schema_name,
       t.typname AS type_name,
       t.typcategory AS type_category
FROM pg_type t
JOIN pg_namespace n ON n.oid = t.typnamespace
WHERE t.typtype = 'e'; -- 'e' indicates enum types

SELECT t.typname AS type_name,
       e.enumlabel AS value
FROM pg_enum e
JOIN pg_type t ON e.enumtypid = t.oid;

SELECT * FROM message WHERE Id_channel = 1;



SELECT c.name_channel, COALESCE(u.pseudo, 'Anonymous user'), m.date_message, m.content
FROM message AS m
JOIN channel AS c ON c.Id_channel = m.Id_channel
LEFT JOIN client AS u ON u.Id_client = m.Id_client
WHERE c.is_private = FALSE AND c.Id_channel = (
    SELECT Id_channel FROM channel
    WHERE name_channel = 'General_chat'
)
ORDER BY date_message;

SELECT c.channel_title, COALESCE(u.user_name, 'Anonymous user'), m.date_time, m.content
FROM messages AS m
JOIN channels AS c ON c.channel_id = m.channel_id
LEFT JOIN users AS u ON u.user_id = m.user_id
WHERE c.channel_status = 'private'
ORDER BY date_time;

SELECT c.channel_title, COALESCE(u.user_name, 'Anonymous user'), m.date_time, m.content
FROM messages AS m
JOIN channels AS c ON c.channel_id = m.channel_id
LEFT JOIN users AS u ON u.user_id = m.user_id
WHERE c.channel_id IN (
    SELECT channel_id FROM channels
    WHERE channel_status = 'private'
) AND u.user_id IN (
    SELECT user_id
    FROM channels_access
    WHERE role_title != 'ban' AND channel_id = 3
);

-- get private message knowing channel id and user_id
SELECT c.channel_title, COALESCE(u.user_name, 'Anonymous user'), m.date_time, m.content
FROM messages AS m
JOIN channels AS c ON c.channel_id = m.channel_id
LEFT JOIN users AS u ON u.user_id = m.user_id
WHERE c.channel_id = 3 AND u.user_id = 3;
ORDER BY date_time;
SELECT user_name, access_id, user_id, channel_id, role_title
FROM channels_access
JOIN users u USING (user_id)
WHERE user_name = 'florence';
SELECT * FROM channels_access;

SELECT channel_id, channel_title, channel_status, ca.role_title
FROM channels
JOIN channels_access ca USING(channel_id)
WHERE user_id = 3;

UPDATE channels_access
SET role_title = 'member'
WHERE user_id = 3 AND channel_id = 3;

-- MATCH AGAINST
SELECT * FROM channels_access;
SELECT * FROM channels;
DESCRIBE users;
SELECT * FROM users;
SELECT * FROM messages;

SELECT CURRENT_USER;