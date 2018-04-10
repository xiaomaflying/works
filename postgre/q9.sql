
create or replace function Q9(_objid integer)
	returns setof text
as $$
declare
	rec AcObjRecord;
	gt text;
	gdef text;
	table char(20);
	_definition text;
	_patterns text array;
	_pattern text;
begin
	select gtype into gt from acad_object_groups where id = _objid;
	if (not found) then
		raise EXCEPTION 'Invalid object id %', _objid;
	end if;

	select gdefby into gdef from acad_object_groups where id = _objid;
	
	if (gdef != 'pattern') then
		return query select null, null where false;
	else
	 	if (gt = 'subject') then
	 		select definition into _definition from acad_object_groups where id = _objid;
	 		_patterns = regexp_split_to_array(_definition, ',');
	 		for i in 1 .. array_upper(_patterns, 1)
	 		loop
	 			return next _patterns[i];
	 		end loop;
	 		-- raise EXCEPTION 'value %', _patterns;
	 	elsif (gt = 'program') then
	 		raise EXCEPTION 'program';
	 	elsif (gt = 'stream') then
	 		raise EXCEPTION 'stream';
	 	end if;
		rec = (gt, gdef);
		-- return next rec;
	end if;
end;
$$ language plpgsql
;


-- select regexp_split_to_array('abc,bcd', ',');

-- select replace('###COMP###', '#', '.');