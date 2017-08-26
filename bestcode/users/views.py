import os
from PIL import Image

from django.shortcuts import render
from django.contrib.auth import authenticate
from django.contrib.auth import login as auth_login
from django.contrib.auth import logout as auth_logout
from django.contrib.auth import models as auth_models
from django.http import HttpResponseRedirect

from bestcode import breadcrumbs
from comment import models as comment_models
from submit import models as submit_models
from activities import models as activity_models
from . import models

def user(request, user_name):
	# comments to me
	comments = comment_models.Comment.objects.filter(comment_type__comment_type_name='user').filter(object_id=user_name)

	# breadcrumb
	breadcrumbs.JumpTo(request, user_name, request.get_full_path())
	request.session.modified = True

	# as an attendant for the submits
	user_submits = submit_models.Submit.objects.filter(activity_user__user__username=user_name)

	# as a reviewer for the submits
	reviewer_submits = submit_models.Submit.objects.filter(submit_reviewer__user__username=user_name)
	
	# comments posted by me
	my_comments = comment_models.Comment.objects.filter(comment_author__user__username=user_name)
	for my_comment in my_comments:
		if my_comment.comment_type.comment_type_name == "main":
			my_comment.target_name = "首页"
			my_comment.target_url = "/main"
		elif my_comment.comment_type.comment_type_name == "activity":
			my_comment.target_name = activity_models.Activity.objects.get(activity_id=my_comment.object_id).name 
			my_comment.target_url = "/activities/" + my_comment.object_id
		elif my_comment.comment_type.comment_type_name == "submit":
			my_comment.target_name = submit_models.Submit.objects.get(submit_id=my_comment.object_id)
			my_comment.target_url = "/submit/" + my_comment.object_id
		elif my_comment.comment_type.comment_type_name == "user":
			my_comment.target_name = my_comment.object_id
			my_comment.target_url = "/user/" + my_comment.object_id

	# get user object
	try:
		auth_user = auth_models.User.objects.get(username=user_name)
		acti_user = models.ActivityUser.objects.get(user=auth_user)
	except:
		return render(request, 'comment/500.html')

	context = {
		'nav_items': breadcrumbs.GetNavItems(request),
		'login': request.user.is_authenticated,
		'acti_user': acti_user,
		'user_submits': user_submits,
		'reviewer_submits': reviewer_submits,
		'comments': comments,
		'my_comments': my_comments,
		'comment_path': "/comment/?comment_type=user&object_id=%s&next='/users/%s'" % (user_name, user_name),
	}
	return render(request, "users/user.html", context)

def logout(request):
	auth_logout(request)
	return HttpResponseRedirect('/main')

def login(request):
	if request.method == 'GET':
		if request.path == '/':
			return HttpResponseRedirect("users/login.html?next=/main")
		else:
			breadcrumbs.JumpTo(request, "登录", request.get_full_path())
			request.session.modified = True
			context = {
				'nav_items': breadcrumbs.GetNavItems(request),
			}

			return render(request, "users/login.html", context)
	elif request.method == 'POST':
		user_name = request.POST['user_name']
		user_pwd = request.POST['user_pwd']
		user = authenticate(request=request, username=user_name, password=user_pwd)
		if user is not None:
			auth_login(request, user)
			if 'next' in request.GET:
				return HttpResponseRedirect(request.GET['next'])
			else:
				return HttpResponseRedirect('/main')
		else:
			return render(request, "users/login.html", 
				{"login_error": "用户名或密码错误",
				"reg_path": "./register"})

def __getContext(request, error):
	context = {
		'nav_items': breadcrumbs.GetNavItems(request),
		'primary_deps': models.PrimaryDepartment.objects.all(),
		'secondary_deps': models.SecondaryDepartment.objects.all(),
		'programming_lans': models.ProgrammingLanguage.objects.all(),
		'reg_error': error
	}
	return context
	
def showError(request, error):
	return render(request, "users/register.html", __getContext(request, error)) 

def register(request):
	if request.method == 'GET':
		breadcrumbs.JumpTo(request, "注册", request.get_full_path())
		request.session.modified = True
		return render(request, "users/register.html", __getContext(request, ""))
	elif request.method == 'POST':
		user_name = request.POST['user_name']
		chinese_name = request.POST['chinese_name']
		primary_dep = request.POST['primary_dep']
		secondary_dep = request.POST['secondary_dep']
		email = request.POST['email']
		programming_lans = request.POST.getlist('programming_lan')
		password = request.POST['password']
		confirm_pwd = request.POST['confirm_pwd']
		
		# user photo
		try:
			user_photo = request.FILES.get('user_photo')
			user_photo_img = Image.open(user_photo)
			new_photo_name = '%s%s' % (user_name, os.path.splitext(user_photo.name)[1])
			user_photo_img.save('media/users/photos/%s' % new_photo_name)
		except Exception as e:
			return showError(request, e)

		# If user exists
		if auth_models.User.objects.filter(username=user_name):
			return showError(request, "用户已存在")

		# Password confirm error?
		if password != confirm_pwd:
			return showError(request, "密码确认错误")

		# Create auth user
		user = auth_models.User.objects.create_user(user_name, email, password)
	   
		try:
			# Create activity user
			acti_user = models.ActivityUser.objects.create(user=user,
				chinese_name=chinese_name, 
				primary_department=models.PrimaryDepartment.objects.get(
					primary_department_id=primary_dep), 
				secondary_department=models.SecondaryDepartment.objects.get(
					secondary_department_id=secondary_dep),
				photo=('/media/users/photos/%s' % new_photo_name))

			# add programming languages
			for language in programming_lans:
				if language == '1':
					acti_user.programming_languages.add(models.ProgrammingLanguage.objects.get(language_name="C++"))
				elif language == '2':
					acti_user.programming_languages.add(models.ProgrammingLanguage.objects.get(language_name="Java"))
				elif language == '3':
					acti_user.programming_languages.add(models.ProgrammingLanguage.objects.get(language_name="Python"))
				elif language == '4':
					acti_user.programming_languages.add(models.ProgrammingLanguage.objects.get(language_name="JS"))
				elif language == '5':
					acti_user.programming_languages.add(models.ProgrammingLanguage.objects.get(language_name="OC"))
				elif language == '6':
					acti_user.programming_languages.add(models.ProgrammingLanguage.objects.get(language_name="Swift"))
				else:
					print("Invalid language")
			acti_user.save()

			# Login user
			auth_login(request, user)
			
			# Go to main page	
			return HttpResponseRedirect("/main/")
		except:
			user.delete()
			# FIXME: should be 500.html
			return HttpResponseRedirect("/main/") 


