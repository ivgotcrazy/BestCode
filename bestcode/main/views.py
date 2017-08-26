import os
from django.shortcuts import render
from django.contrib.auth.decorators import login_required
from comment import models as comment_models
from activities import models as activity_models
from bestcode import breadcrumbs
from . import models

def main(request):
	comments = comment_models.Comment.objects.filter(comment_type__comment_type_name='main')
	activities = activity_models.Activity.objects.all()

	breadcrumbs.Clear(request)
	breadcrumbs.JumpTo(request, '首页', request.get_full_path())

	# banner
	banners = models.Banner.objects.all()

	# rules
	rules = models.Rule.objects.all()

	# news
	newses = models.News.objects.all()

	context = { 
		'nav_items': breadcrumbs.GetNavItems(request),
		'login': request.user.is_authenticated,
		'activities': activities,
		'comments': comments,
		'banners': banners,
		'rules': rules,
		'newses': newses,
		'comment_path': '/comment/?comment_type=main&object_id=0&next=\'/main\''
 	}
	return render(request, 'main/main.html', context)
