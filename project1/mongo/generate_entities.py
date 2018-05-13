from faker import Faker

fake = Faker()

def fake_users(number):
	users = []
	for _ in range(number):
		# email, firstname, lastname, phone, addr1, addr2, city, state, zipcode
		email = fake.email()
		firstname = fake.first_name()
		lastname = fake.last_name()
		phone = fake.phone_number()
		addr1 = fake.address()
		addr2 = fake.address()
		city = fake.city()
		state = fake.state()
		zipcode = fake.zipcode()
		info = {
			'email': email,
			'firstname': firstname,
			'lastname': lastname,
			'phone': phone,
			'address_line1': addr1,
			'address_line2': addr2,
			'city': city,
			'state': state,
			'zip': zipcode
		}
		users.append(info)
	return users


def fake_user_sql(number=1000):
	users = fake_users(number)
	sqls = []
	for user in users:
		sql = 'INSERT INTO users'\
			'(email, firstname, lastname, phone, address_line1, address_line2, city, state, zip) VALUES '\
			"('{email}', '{firstname}', '{lastname}', '{phone}', '{address_line1}', '{address_line2}', '{city}', '{state}', '{zip}');"\
			.format(**user)
		sqls.append(sql)
	return '\n'.join(sqls)


def fake_recipe_data(number):
	data = []
	my_word_list = [
		'danish','cheese','sugar',
		'Lollipop','wafer','Gummies',
		'sesame','Jelly','beans',
		'pie','bar','Ice','oat'
	]
	words = my_word_list[:]
	words.extend(['%s1' % i for i in my_word_list])
	words.extend(['%s2' % i for i in my_word_list])

	for index in range(number):
		data.append({
			'name': words[index],
			'description': fake.sentence(ext_word_list=my_word_list),
			'instructions': fake.text()
		})
	return data


def fake_recipe_sql(number=25):
	recipes = fake_recipe_data(number)
	sqls = []
	for recipe in recipes:
		sql = 'INSERT INTO recipes'\
			'(name, description, instructions) VALUES '\
			"('{name}', '{description}', '{instructions}');"\
			.format(**recipe)
		sqls.append(sql)
	return '\n'.join(sqls)


def fake_order_data(user_num, recipe_num, number):
	import random
	data = []
	for _ in range(number):
		data.append({
			'user_id': random.choice(range(1, user_num+1)),
			'recipe_id': random.choice(range(1, recipe_num+1)),
			'create_time': str(fake.date_time_between(start_date='-1y'))
		})
	return data

def fake_order_sql(user_num=1000, recipe_num=25, number=50000):
	orders = fake_order_data(user_num, recipe_num, number)
	sqls = []
	for order in orders:
		sql = 'INSERT INTO orders'\
			'(user_id, recipe_id, create_time) VALUES '\
			"({user_id}, {recipe_id}, '{create_time}');"\
			.format(**order)
		sqls.append(sql)
	return '\n'.join(sqls)


def test():
	print(fake_order_sql(2, 2, 2))


def main():
	user_num = 1000
	recipe_num = 25
	order_num = 50000
	with open('generate.sql', 'w') as f:
		f.write('-- fake users data\n')
		f.write(fake_user_sql(user_num))
		f.write('\n-- fake recipes data\n')
		f.write(fake_recipe_sql(recipe_num))
		f.write('\n-- fake orders data\n')
		f.write(fake_order_sql(user_num, recipe_num, order_num))
	

if __name__ == '__main__':
	# test()
	main()


