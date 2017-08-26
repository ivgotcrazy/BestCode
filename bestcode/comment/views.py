import datetime
from django.shortcuts import render
from comment import models as comment_models
from users import models as user_models
from django.contrib.auth import models as auth_models
from bestcode import breadcrumbs
from django.http import HttpResponse
from . import models

	
def comment(request):
	try:
		comment_type = request.GET['comment_type']
		object_id = request.GET['object_id']
	except:
		return render(request, 'comment/500.html') 

	if request.method == 'GET':
		# set breadcrumbs nav
		breadcrumbs.JumpTo(request, "评论", request.get_full_path())
		return __getResponse(request, comment_type, object_id)
	elif request.method == 'POST':
		try:
			auth_user = auth_models.User.objects.get(username=request.user)
			comment_user = user_models.ActivityUser.objects.get(user=auth_user)
		except:
			return render(request, 'comment/500.html')

		# create comment and save to db
		comment = models.Comment.objects.create(
			comment_author		= comment_user,
			comment_type		= comment_models.CommentType.objects.get(comment_type_name=comment_type),
			object_id			= object_id,
			comment_text		= request.POST['comment_text'],
			comment_time		= datetime.datetime.now(),
			agree_count			= 0,
			disagree_count		= 0,
			reply_of_comment_id	= 0)
		comment.save()

		return __getResponse(request, comment_type, object_id)

def agree(request):
	comment_id = request.GET['comment_id']
	agree_type = request.GET['type']
	
	try:
		comment = models.Comment.objects.get(comment_id=comment_id)
	except:
		return HttpResponse("failed")

	if "comment_agree" not in request.session:
		request.session['comment_agree'] = [] 

	comment_agree_list = request.session['comment_agree']

	if comment_agree_list.count(comment_id) > 0:
		return HttpResponse("failed")
	else:
		comment_agree_list.append(comment_id)
		request.session.modified = True

		if agree_type == 'agree':
			comment.agree_count += 1
		else:
			comment.disagree_count += 1
		comment.save()

		return HttpResponse("success")

def __getResponse(request, comment_type, object_id):
	comments = models.Comment.objects.filter(comment_type__comment_type_name=comment_type).filter(object_id=object_id)
	context = {
		'login': request.user.is_authenticated,
		'user_name': request.user.username,
		'nav_items': breadcrumbs.GetNavItems(request),
		'comments': comments,
		'next': request.GET['next']
		}
	return render(request, 'comment/comment.html', context)
