dropdb pizza
createdb pizza
psql -d pizza < create.sql
psql -d pizza < server_function.sql
psql -d pizza < generate.sql
