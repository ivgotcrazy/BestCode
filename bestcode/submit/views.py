from django.shortcuts import render
from django.http import HttpResponse
from bestcode import breadcrumbs
from . import models
from comment import models as comment_models

def submit(request, submit_id):
	user_submit = models.Submit.objects.get(submit_id=submit_id);
	user_name = user_submit.activity_user.user.username;
	comments = comment_models.Comment.objects.filter(comment_type__comment_type_name='submit').filter(object_id=submit_id)

	# browse times
	user_submit.browse_times += 1
	user_submit.save()

	# code description file
	code_desc_file = {}
	try:
		code_desc_file = models.SubmitFile.objects.filter(submit_id=submit_id).get(submit_file_type__submit_file_type_name='desc')
		#code_desc_file = 2
	except:
		pass

	# source code files
	src_code_files = []
	try:
		src_code_files = models.SubmitFile.objects.filter(submit_id=submit_id).filter(submit_file_type__submit_file_type_name='code') 
	except:
		pass

	# code review result file
	code_review_file = {}
	try:
		code_review_file = models.SubmitFile.objects.filter(submit_id=submit_id).get(submit_file_type__submit_file_type_name='review')
	except:
		pass

	# code comment file
	code_comment_file = {}
	try:
		code_comment_file = models.SubmitFile.objects.filter(submit_id=submit_id).get(submit_file_type__submit_file_type_name="comment")
	except:
		pass

	# breadcrumbs
	breadcrumbs.JumpTo(request, ('%s的提交' % user_name), request.get_full_path())
	request.session.modified = True

	# this is a ugly adaption to absolute path and relative path, i just dont want to explain why.
	if user_submit.activity_user.photo.__str__()[0] != '/':
		user_submit.activity_user.photo = ("/media/%s" % user_submit.activity_user.photo)
	if user_submit.submit_reviewer.photo.__str__()[0] != '/':
		user_submit.submit_reviewer.photo = ("/media/%s" % user_submit.submit_reviewer.photo)
	
	context = {
		'nav_items': breadcrumbs.GetNavItems(request),
		'login': request.user.is_authenticated,
		'submit': user_submit,
		'submit_user': user_submit.activity_user,
		'reviewer': user_submit.submit_reviewer,
		'code_desc_file': code_desc_file,
		'code_review_file': code_review_file,
		'code_comment_file': code_comment_file,
		'src_code_files': src_code_files,
		'comments': comments,
		'comment_path': "/comment/?comment_type=submit&object_id=%s&next='/submit/%s'" % (submit_id, submit_id),
		}
	return render(request, "submit/submit.html", context)

def addPraise(request):
	submit_id = request.GET['submit_id']

	try:
		submit = models.Submit.objects.get(submit_id=submit_id)
	except:
		return HttpResponse("failed")

	if 'submit_praise' not in request.session:
		request.session['submit_praise'] = []

	submit_praise_list = request.session['submit_praise']
	
	if submit_praise_list.count(submit_id) > 0:
		return HttpResponse("failed")
	else:
		submit_praise_list.append(submit_id)
		request.session.modified = True

		submit.praise_count += 1
		submit.save()
		return HttpResponse("success")
