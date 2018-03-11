from DB import DaBa # a simple abstraction on sqlite3
from bottle import route, run, debug, template, request, static_file, redirect
import json
# import sqlite3

''' packing and unpacking fetch request and response data  '''

def body2str(req):
    ''' decodes request body into string '''
    return req._get_body_string().decode()
    # would req.text() work instead?
    # inverse to bytes.decode() is string.encode()

def pack_tasks(tasks):
    ''' jsonify view tasks 
     
        >>> json.dumps({'a': 1, 'b':2})
        '{"a": 1, "b": 2}'
    '''
    return json.dumps(tasks)

def unpack_tasks(jsontasks):
    ''' convert json string into view tasks 

        >>> json.loads('{"a": 1, "b": 2}')
        {'a': 1, 'b': 2}
    '''
    return json.loads(jsontasks)

def pack_task(task):
    ''' jsonify view task '''
    return json.dumps(task)

def unpack_task(requ):
    return json.loads(body2str(request))

''' conversion between db model and view model '''

def view_to_db_status(status):
    ''' converts view task status to db task status '''
    if status == "tbd":
        return 0
    elif status == "done":
        return 1
    else:
        print("unknown task status: " + status)
        return "unknown"

def db_to_view_status(status):
    if status == 0:
        return "tbd"
    elif status == 1:
        return "done"
    else:
        print("unknown task status: " + str(status))
        return "unknown"

def db_to_view_task(row):
    ''' convert db row to view task '''
    id, d, s = row
    return {'taskid': id, 'taskdescription': d, 'status': db_to_view_status(s)}

def view_to_db_task(task):
    ''' convert view task to db row '''
    return (task['taskid'], task['taskdescription'], view_to_db_status(task['status']))


def view_to_db_new_task(task):
    ''' convert view new task to db row '''
    return (task['taskdescription'], view_to_db_status(task['status']))


def db_to_view_tasks(rows):
    ''' convert todo table rows into view model tasks '''
    # tasks = [{'taskid': row[0], 'taskdescription': row[1], 'status': row[2]} for row in rows]
    tasks = list(map(db_to_view_task, rows)) # to get list instead of an iterator
    return tasks

def view_to_db_tasks(tasks):
    ''' convert view tasks to db rows '''
    rows = list(map(view_to_db_task, tasks))
    return rows


''' ----------------- db model ------------------- '''
def db_get_tasks_by_status(filter):
    ''' retrieve tasks that pass filter '''
     # connect to todo.db
    db = DaBa("todo")
    # construct query to retrieve rows from todo table
    query = "SELECT id, task, status FROM todo "
    
    if filter == 0:
        query += "WHERE status=0"
    elif filter == 1:
        query += "WHERE status=1"
    else:  # filter == "all"
        pass
    # execute query
    db.que(query)
    # retrieve query result as Python sequence of tuples
    result = db.all()
    # close db (otherwise db locked upon future access)
    db.close()
    return result

def db_create_task(new_task):
    ''' stores new record in db
        returns new record id
    '''
    # connect to todo.db
    db = DaBa("todo")
    # construct query to retrieve rows from todo table
    query = "INSERT INTO todo (task, status) Values('{td}', {status})"
    query = query.format(td=new_task[0], status=(new_task[1]))
    # execute query
    print("Query: " + query)
    db.que(query)
    result = db.new_id()
    # close db to release lock on database
    db.close()
    return result

def db_get_task(id):
    ''' retrieves record with id '''
    db = DaBa("todo")
    query = "SELECT * FROM todo WHERE id={taskid}".format(taskid=id)
    print("in db_get_task, query: ")
    print(query)
    db.que("SELECT * FROM todo WHERE id={taskid}".format(taskid=id))
    result = db.one()
    db.close()
    return result

def db_update_status(id, status):
    ''' update status for task with id '''
    db = DaBa("todo")
    # note: the following string interpolation happens in SQL, not in Python
    db.que("UPDATE todo SET status = ? WHERE id LIKE ?", (status, id))
    db.close()


''' ----------------- view model ------------------- '''

''' view task model
    task = {'taskid': int, taskdescription: text, status: 'done' or 'tbd'}
'''

def view_get_tasks(filter):
    ''' retrieve tasks '''
    # convert filter
    if filter == "all":
        db_status = "all"
    else:
        db_status = view_to_db_status(filter)
    # get db_tasks
    rows = db_get_tasks_by_status(db_status)
    # convert rows to tasks in view model representation
    result = db_to_view_tasks(rows)
    return result

def view_get_task(taskid):
    return db_to_view_task(db_get_task(taskid))


def view_new_task(new_task):
    ''' create new task:
        store task in db
        retrieve, convert to view, and return stored task
    '''
    # convert task data from vm to db 
    db_new_task = view_to_db_new_task(new_task)
    # create record in db
    taskid = db_create_task(db_new_task)
    # confirming record was correctly added
    row = db_get_task(taskid)
    result = db_to_view_task(row) if row else {'error': "task insertion failed"}
    return result

def view_update_status(vm_task_data):
    ''' updates status to vm_status for task with taskid '''
    taskid = vm_task_data['taskid']
    vm_status = vm_task_data['status']
    db_status = view_to_db_status(vm_status)
    db_id = taskid
    db_update_status(db_id, db_status)
    #confirming update
    task = view_get_task(taskid)
    if vm_status == task['status']:
        return {'taskid': taskid, 'status': vm_status}
    else:
        return {'error': "status update failed"}



@route('/')
@route('/index')
@route('/home')
def index():
    ''' returns index page '''
    return template('todospa.tpl')
    # note on todospa.tpl above:
    # my template file names end in .tpl.html
    # .tpl indicates it is a template
    # .html makes my editor's syntax support kick in

@route('/tasks/<filter>')
def get_tasks(filter):
    ''' retrieve filtered tasks from db
        and return as json sequence
    '''
    tasks = view_get_tasks(filter)
    return pack_tasks(tasks)

@route('/task/new', method='POST')
def new_task():
    ''' record a new task '''
    new_task = unpack_task(request)
    task = view_new_task(new_task)
    return pack_task(task)


@route('/status/update', method='POST')
def update_status():
    ''' update task status '''
    # decode json message body into Python dictionary
    task_data = unpack_task(request) 

    # you may remove or comment out the print statements
    print("from update_status: ")
    print(task_data)

    updated_task = view_update_status(task_data)

    print(updated_task)

    return pack_task(updated_task)

@route('/static/<filename>')
def static(filename):
    if filename:
        return static_file(filename, root='./static')
    else:
        return '<script>alert("File name is missing")</script>' 

# during development (not in production)
debug(True)

# start bottle webserver on localhost:8080
# reload script after change (without restarting server)
run(reloader=True)
