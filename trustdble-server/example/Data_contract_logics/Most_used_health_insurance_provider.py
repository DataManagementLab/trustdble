import sys
import mysql.connector
import re
def main():
  connection = mysql.connector.connect(host="localhost",user="root", password="",database="Hospital_Digital_System")
  sql_select_Query = "SELECT patients.health_insurance_provider, COUNT(*) AS total FROM patients GROUP BY health_insurance_provider ORDER BY total DESC"
  cursor = connection.cursor()
  cursor.execute(sql_select_Query)
  records = cursor.fetchall()
  connection.close()
  cursor.close()
  return records