#!/home/sudharsan/myenv/bin/python3
import mysql.connector
from dotenv import load_dotenv
import os

# Load environment variables
load_dotenv()
db_name = os.getenv('DB_NAME')
pass_key = os.getenv('DB_PASSWORD')

def get_db_connection():
    """Get a new database connection."""
    return mysql.connector.connect(
        host="localhost",
        user="root",
        password=pass_key,
        database=db_name
    )

def check(mac):
    connection = get_db_connection()
    cursor = connection.cursor()
    try:
        query = "SELECT * FROM registered_mac WHERE mac_id=%s"
        cursor.execute(query, (mac,))
        result = cursor.fetchone()
        return result is not None
    finally:
        cursor.close()
        connection.close()

def update(username, new_name, new_passcode, new_system):
    connection = get_db_connection()
    cursor = connection.cursor()
    print('update selected')
    try:
        query = "UPDATE registered_mac SET user_name=%s, pass_code=%s WHERE user_name=%s"
        cursor.execute(query, (new_name, new_passcode, username))
        connection.commit()

        query = "UPDATE details SET user_name=%s, system=%s WHERE user_name=%s"
        cursor.execute(query, (new_name, new_system, username))

        connection.commit()
        
    except Exception as e:
        connection.rollback()
        print("Error occurred:", e)
    finally:
        cursor.close()
        connection.close()


def login(username, passcode):
    connection = get_db_connection()
    cursor = connection.cursor()
    try:
        query = "SELECT * FROM registered_mac WHERE user_name=%s AND pass_code=%s"
        cursor.execute(query, (username, passcode))
        result = cursor.fetchone()
        return result is not None
    finally:
        cursor.close()
        connection.close()
        
def profile(name):
    connection = get_db_connection()
    cursor = connection.cursor()
    try:
        query = '''
        SELECT * FROM registered_mac 
        INNER JOIN details ON registered_mac.user_name = details.user_name 
        WHERE registered_mac.user_name = %s;
        '''
        print(f"Executing query: {query} with name: {name}")
        cursor.execute(query, (name,))
        result = cursor.fetchone()
        print(f"Query result: {result}")
        if result:
            dataset = {
                'name': result[0],
                'year': result[5],
                'course': result[4],
                'section': 'D',
                'mac_id': result[2],
                'roll-no': result[0],
                'pass_code': result[1],
                'sys_name':result[6],
                'os':result[7]
            }
            return dataset
        else:
            print("No matching records found.")
            return None
    except Exception as e:
        print(f"An error occurred: {e}")
        return None
    finally:
        cursor.close()
        connection.close()


if __name__ == "__main__":
    data = login('22csr207', 'kongu@2024')
    print(data)
