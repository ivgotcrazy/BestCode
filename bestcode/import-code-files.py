import MySQLdb
import sys, getopt, os


def PrintHelpAndExit():
	help_msg = '''Usage: python import-code-files.py -f <file-path> -s <submit-id>
	-h print help info
	-u user name
	-f file path
	-s submit id'''
	print(help_msg)
	sys.exit(1)

def GetParam(argv):
	file_path = ""
	submit_id = ""
	user_name = ""
	try:
		opts, args = getopt.getopt(argv, "hu:f:s:", ["user-name=","file-path=","submit-id="])
	except getopt.GetoptError:
		PrintHelpAndExit()

	if len(opts) == 0:
		PrintHelpAndExit()

	for opt, arg in opts:
		if opt == '-h':
			PrintHelpAndExit()
		elif opt in ("-u", "--user-name"):
			user_name = arg
		elif opt in ("-f", "--file-path"):
			file_path = arg
		elif opt in ("-s", "--submit-id"):
			submit_id = arg
		else:
			print("Invalid option '%s'" % opt)
			sys.exit(1)

	return file_path, submit_id, user_name


def Main(argv):
	file_path, submit_id, user_name = GetParam(argv)
	
	try:
		conn = MySQLdb.connect(host="localhost", port=3306, user='root', passwd='')
		conn.select_db('bestcode')
	except:
		print("Connect to DB failed")
		sys.exit(1)

	for dirpath, dirnames, filenames in os.walk(file_path):
		for filename in filenames:
			full_path = ("/media/activities/2017Q3/submits/%s/code/%s" % (user_name, filename))
			show_file_name = os.path.splitext(filename)[0]
			exec_str = ("insert into submit_submitfile(submit_id, file_path, submit_file_type_id, file_name) values (%s, '%s', %d, '%s')" % (submit_id, full_path, 2, show_file_name))
			print(conn.cursor().execute(exec_str))
			print(exec_str)
	conn.commit()
	conn.close()

if __name__ == '__main__':
	Main(sys.argv[1:])
