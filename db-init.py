import MySQLdb
import sys, getopt


DATABASE = 'bestcode'
DATABASE_USER = 'bestcode-admin'

def PrintHelpAndExit():
	help_msg = '''Usage: python db-init.py -u <user-name> -p <password> -f
	-h print help info
	-u user to login mysql
	-p user password
	-f whether to remove database if database exists'''
	print(help_msg)
	sys.exit(1)

def GetLoginDbParam(argv):
	user_name = ""
	password = ""
	force_rm_db = False

	try:
		opts, args = getopt.getopt(argv, "hu:p:f", ["user=","password="])
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
		elif opt in ("-f", "--force-rm-db"):
			force_rm_db = True
		else:
			print("Invalid option '%s'" % opt)
			sys.exit(1)

	return user_name, password, force_rm_db


def CreateDatabase(argv):
	user_name, password, force_rm_db = GetLoginDbParam(argv)
	conn = MySQLdb.connect(host="localhost", port=3306, user=user_name, passwd=password)

	# create database
	try:
		conn.select_db('%s' % DATABASE)
		print(">>> Database '%s' already exists." % DATABASE)
		if force_rm_db:
			print(">>> Start to remove database %s." % DATABASE)
			conn.cursor().execute("drop database %s" % DATABASE)
			print("<<< Removed database %s." % DATABASE)
			raise
	except:
		conn.cursor().execute("create database if not exists %s" % DATABASE)
		print(">>> Create '%s' database success." % DATABASE)

	# delete user first.
	try:
		# to be noticed, delete user from mysql.user does not totally equal drop user
		conn.cursor().execute("drop user '%s'@'%%'" % DATABASE_USER)
		conn.cursor().execute("drop user '%s'@'localhost'" % DATABASE_USER)
	except:
		print("<<< User '%s' dose not exist." % DATABASE_USER)

	# create user and make grant 
	conn.cursor().execute("create user '%s'@'%%' identified by '123456'" % DATABASE_USER)
	conn.cursor().execute("grant all privileges on %s.* to '%s'@'%%'" % (DATABASE, DATABASE_USER))
	
	# must create localhost acount which allows localhost to login, but why '%' does not work?
	conn.cursor().execute("create user '%s'@'localhost' identified by '123456'" % DATABASE_USER)
	conn.cursor().execute("grant all privileges on %s.* to '%s'@'localhost'" % (DATABASE, DATABASE_USER))

	conn.cursor().execute("flush privileges")
	print(">>> Create user '%s' success." % DATABASE_USER)

	conn.close()

if __name__ == '__main__':
	CreateDatabase(sys.argv[1:])
