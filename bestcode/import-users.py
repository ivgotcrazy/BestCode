import MySQLdb
import sys, getopt


DATABASE = 'bestcode'
DATABASE_USER = 'bestcode-admin'

def PrintHelpAndExit():
	help_msg = '''Usage: python import-users.py -u <user-name> -p <password> -f <users-file>
	-h print help info
	-u user to login mysql
	-p user password
	-f user list '''
	print(help_msg)
	sys.exit(1)

def GetLoginDbParam(argv):
	user_name = ""
	password = ""
	force_rm_db = False

	try:
		opts, args = getopt.getopt(argv, "hu:p:f:", ["user=", "password=", "users-file"])
	except getopt.GetoptError:
		PrintHelpAndExit()

	if len(opts) == 0:
		PrintHelpAndExit()

	for opt, arg in opts:
		if opt == '-h':
			PrintHelpAndExit()
		elif opt in ("-u", "--user-name"):
			user_name = arg
		elif opt in ("-p", "--password"):
			password = arg
		elif opt in ("-f", "--users-file"):
			config = arg
		else:
			print("Invalid option '%s'" % opt)
			sys.exit(1)

	return user_name, password, config


def ImportNames(argv):
	user_name, password, config = GetLoginDbParam(argv)

	try:
		conn = MySQLdb.connect(host="localhost", port=3306, user=user_name, passwd=password)
		conn.select_db('%s' % DATABASE)
	except:
		print("!!! Failed to connect database.")
		return

	try:
		names_file = open(config, 'r')
	
		for line in names_file:
			if len(line.strip()) != 0:
				print(line.split())	
	except:
		print("!!! Failed to read file.")
		return


	'''
	# create user and make grant 
	conn.cursor().execute("create user '%s'@'%%' identified by '123456'" % DATABASE_USER)
	conn.cursor().execute("grant all privileges on %s.* to '%s'@'%%'" % (DATABASE, DATABASE_USER))
	
	# must create localhost acount which allows localhost to login, but why '%' does not work?
	conn.cursor().execute("create user '%s'@'localhost' identified by '123456'" % DATABASE_USER)
	conn.cursor().execute("grant all privileges on %s.* to '%s'@'localhost'" % (DATABASE, DATABASE_USER))

	conn.cursor().execute("flush privileges")
	print(">>> Create user '%s' success." % DATABASE_USER)

	'''

	conn.close()

if __name__ == '__main__':
	ImportNames(sys.argv[1:])
