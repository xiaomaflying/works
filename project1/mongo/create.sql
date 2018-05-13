CREATE TABLE users (
	id SERIAL PRIMARY KEY,
	email varchar(100) NOT NULL,
	firstname varchar(100) NOT NULL,
	lastname varchar(100) NOT NULL,
	phone varchar(100) NOT NULL,
	address_line1 varchar(100),
	address_line2 varchar(100),
	city varchar(100),
	state varchar(100),
	zip varchar(20)
);

CREATE TABLE ingredients (
	id SERIAL PRIMARY KEY,
	name varchar(100) NOT NULL,
	description varchar(512),
	quantity INTEGER DEFAULT 0 NOT NULL
);

CREATE TABLE recipes (
	id SERIAL PRIMARY KEY,
	name varchar(100) NOT NULL,
	description varchar(512),
	instructions text
);

CREATE TABLE recipe_ingredient (
	recipe_id INTEGER REFERENCES recipes(id) NOT NULL,
	ingredient_id INTEGER REFERENCES ingredients(id) NOT NULL,
	num INTEGER NOT NULL,
	PRIMARY KEY (recipe_id, ingredient_id)
);

CREATE TABLE orders (
	id SERIAL PRIMARY KEY,
	user_id INTEGER REFERENCES users(id) NOT NULL,
	recipe_id INTEGER REFERENCES recipes(id) NOT NULL,
	create_time TIMESTAMP NOT NULL DEFAULT NOW()
);

