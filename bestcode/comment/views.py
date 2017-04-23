from django.shortcuts import render
from . import models
from comment import models as comment_models
import datetime

def __getResponse(request):
	comments = models.Comment.objects.filter(comment_type__comment_type_name=request.GET['comment_type'])
	context = {
		'login': request.user.is_authenticated,
		'user_name': request.user.username,
		'nav_items': [{'path': 'activities', 'text': '评论'}],
		'comments': comments,
		}
	return render(request, 'comment/comment.html', context)
	
def comment(request):
	if not request.GET.__contains__('comment_type') or not request.GET.__contains__('object_id'):
		return render(request, 'comment/500.html') 

	if request.method == 'GET':
		return __getResponse(request)
	elif request.method == 'POST':
		print(request.GET['comment_type'])
		comment = models.Comment.objects.create(
			comment_author=request.user,
			comment_type=comment_models.CommentType.objects.get(comment_type_name=request.GET['comment_type']),
			object_id=0,
			comment_text=request.POST['comment_text'],
			comment_time=datetime.datetime.now(),
			agree_count=0,
			disagree_count=0,
			reply_of_comment_id=0)

		comment.save()
		return __getResponse(request)
		

