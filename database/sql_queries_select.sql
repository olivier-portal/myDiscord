SELECT table_name 
FROM information_schema.tables 
WHERE table_schema = 'public';

SELECT n.nspname AS schema_name,
       t.typname AS type_name,
       t.typcategory AS type_category
FROM pg_type t
JOIN pg_namespace n ON n.oid = t.typnamespace
WHERE t.typtype = 'e'; -- 'e' indicates enum types

SELECT t.typname AS type_name,
       e.enumlabel AS value
FROM pg_enum e
JOIN pg_type t ON e.enumtypid = t.oid;;