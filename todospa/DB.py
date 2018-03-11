
import sqlite3
class DaBa:
    ''' Simple sqlite3 Abstraction '''
    
    def __init__(self, dbname):
        self._conn = sqlite3.connect(dbname + '.db')
        self._cursor = self._conn.cursor()
        
    # def set_cursor(self):
        # self._cursor = self._conn

    def close(self):
        self._conn.commit()
        self._cursor.close()

    def que(self, *query):
        print("From que: ")
        print(*query)
        print("last rowid before query: ")
        print(self._cursor.lastrowid)
        self._cursor.execute(*query)
        print("last rowid after query: ")
        print(self._cursor.lastrowid)

    def one(self):
        return self._cursor.fetchone()
    
    def all(self):
        return self._cursor.fetchall()

    def new_id(self):
        return self._cursor.lastrowid



