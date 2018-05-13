create or replace function create_order(_user_id integer, _recipe_id integer)
	returns void
as $$
declare
	less_num integer;
	_rec_id integer;
	_after_used integer;
begin
	select count(*) into less_num from ingredients i join recipe_ingredient ri on ri.ingredient_id = i.id 
		join recipes r on r.id=ri.recipe_id where r.id=_recipe_id and i.quantity-ri.num < 0;
	if (less_num > 0) then
		raise EXCEPTION 'there are not enough ingredients';
	end if;

	for _rec_id, _after_used in select i.id, i.quantity-ri.num as after_used from ingredients i 
		join recipe_ingredient ri on ri.ingredient_id = i.id join recipes r 
		on r.id=ri.recipe_id where r.id=_recipe_id
	loop
		update ingredients set quantity = _after_used where id=_rec_id;
	end loop;
	insert into orders (user_id, recipe_id) values (_user_id, _recipe_id);
end;
$$ language plpgsql
;