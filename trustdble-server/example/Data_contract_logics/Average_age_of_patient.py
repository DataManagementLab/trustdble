import sys
import mysql.connector
import re
def main():
  connection = mysql.connector.connect(host="localhost",user="root", password="",database="Hospital_Digital_System")
  sql_select_Query = " SELECT AVG(DATEDIFF(CURDATE(), date_of_birth) / 365) AS avg_age FROM patients"
  cursor = connection.cursor()
  cursor.execute(sql_select_Query)
  records = cursor.fetchall()
  connection.close()
  cursor.close()
  return records