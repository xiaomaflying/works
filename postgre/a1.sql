-- COMP3311 18s1 Assignment 1
-- Written by YOUR_NAME (YOUR_STUDENT_ID), April 2018

-- Q1: ...

create or replace view Q1(unswid, name)
as
select p.unswid, p.name from people p join students s on s.id = p.id join 
(
	select student, count(*) as number from course_enrolments group by student having count(*)>65
) t 
on s.id=t.student;

-- Q2: ...

create or replace view Q2(nstudents, nstaff, nboth)
as
select 
	(select count(*) from students) as nstudents,
	(select count(*) from staff) as nstaff,
	(select count(*) from staff sta join students stu on stu.id=sta.id) as nboth
;

-- Q3: ...

create or replace view name_ncourse(name, ncourses)
as
select p.name as name, t.course_num as ncourses from people p join 
	(select cs.staff, count(*) as course_num from staff_roles sr join course_staff cs on cs.role=sr.id
	 where sr.name='Course Convenor' group by cs.staff) t 
on t.staff=p.id;

create or replace view Q3(name, ncourses)
as
select name, ncourses from name_ncourse where ncourses = (select max(ncourses) from name_ncourse)
;

-- Q4: ...

create or replace view Q4a(id)
as
select peo.unswid as id from program_enrolments pe join semesters se 
	on se.id=pe.semester join programs p on p.id=pe.program join people peo on peo.id=pe.student
	where  se.year='2005' and se.term='S2' and p.code='3978'
;

create or replace view Q4b(id)
as
select peo.unswid as id from program_enrolments pe join semesters se on se.id=pe.semester 
	join programs p on p.id=pe.program 
	join people peo on peo.id=pe.student 
	join stream_enrolments ste on ste.partof=pe.id 
	join streams on streams.id=ste.stream 
	where  se.year='2005' and se.term='S2' and streams.code='SENGA1'
;

create or replace view Q4c(id)
as
select peo.unswid as id from program_enrolments pe join semesters se on se.id=pe.semester join programs p on p.id=pe.program join people peo on peo.id=pe.student join program_group_members pgm on pgm.program=p.id join acad_object_groups aog on aog.id=pgm.ao_group where  se.year='2005' and se.term='S2' and aog.name like '%CSE%' group by peo.unswid;
;

-- Q5: ...

create or replace view faculty_commitee_num(id, num)
as
select ou.id, count(*) from 
	(select id as cid, facultyOf(id) as id from OrgUnits) ou 
	join OrgUnit_groups oug on oug.owner=ou.id 
	join OrgUnits oua on oug.member=oua.id 
	join OrgUnit_types out on out.id=oua.utype 
	where out.name='Committee' group by ou.id order by count(*) desc;

create or replace view Q5(name)
as
select ou.name from faculty_commitee_num fcn join OrgUnits ou on fcn.id =ou.id where num = (select max(num) from faculty_commitee_num);
;

-- Q6: ...

create or replace function Q6(integer) returns text
as
$$
select name from people where id=$1 or unswid=$1
$$ language sql
;

-- Q7: ...

create or replace function Q7(text)
	returns table (course text, year integer, term text, convenor text)
as $$
select cast(sub.code as text) as code, se.year, cast(se.term as text) as term, cast(p.name as text) as convenor from courses c 
	join subjects sub on sub.id=c.subject 
	join semesters se on se.id=c.semester 
	join course_staff cs on cs.course=c.id 
	join people p on p.id=cs.staff 
	join staff_roles sr on sr.id=cs.role where sub.code=$1 and sr.name = 'Course Convenor';
$$ language sql
;

-- Q8: ...

create or replace function Q8(integer)
	returns setof NewTranscriptRecord
as $$
declare
	... PLpgSQL variable delcarations ...
begin
	... PLpgSQL code ...
end;
$$ language plpgsql
;


-- Q9: ...

create or replace function Q9(integer)
	returns setof AcObjRecord
as $$
declare
	... PLpgSQL variable delcarations ...
begin
	... PLpgSQL code ...
end;
$$ language plpgsql
;

