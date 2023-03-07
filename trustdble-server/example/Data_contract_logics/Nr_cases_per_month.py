import sys
import mysql.connector
import re
def main():
  connection = mysql.connector.connect(host="localhost",user="root", password="",database="Hospital_Digital_System")
  sql_select_Query = "SELECT YEAR(admission_date) AS year, MONTH(admission_date) AS month, COUNT(*) AS cases_opened FROM cases GROUP BY YEAR(admission_date), MONTH(admission_date);"
  cursor = connection.cursor()
  cursor.execute(sql_select_Query)
  records = cursor.fetchall()
  connection.close()
  cursor.close()
  return records