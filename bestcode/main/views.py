import os
from django.shortcuts import render
from django.contrib.auth.decorators import login_required
from comment import models as comment_models
from activities import models as activity_models

def main(request):
	comments = comment_models.Comment.objects.filter(comment_type__comment_type_name='main')
	activities = activity_models.Activity.objects.all()

	# Process photo path fro static file
	for activity in activities:
		activity.photo_static_url = 'activities/' + os.path.basename(activity.photo.url)

	context = { 
		'nav_items': [{'path': '.', 'text': '首页'}],
		'login': request.user.is_authenticated,
		'activities': activities,
		'comments': comments,
		'comment_path': '/comment/?comment_type=main&object_id=0'
 	}
	return render(request, 'main/main.html', context)
