from django.shortcuts import render
from django.contrib.auth import authenticate
from django.contrib.auth import login as auth_login
from django.contrib.auth import models as auth_models
from django.http import HttpResponseRedirect
from . import models

def login(request):
	if request.method == 'GET':
		if request.path == '/':
			return HttpResponseRedirect("users/login.html?next=/main")
		else:
			return render(request, "users/login.html")
	elif request.method == 'POST':
		user_name = request.POST['user_name']
		user_pwd = request.POST['user_pwd']
		user = authenticate(request=request, username=user_name, password=user_pwd)
		if user is not None:
			auth_login(request, user)
			return HttpResponseRedirect(request.GET['next'])
		else:
			return render(request, "users/login.html", 
				{"login_error": "用户名或密码错误",
				"reg_path": "./register"})

def __getContext(error):
	context = {
		'primary_deps': models.PrimaryDepartment.objects.all(),
		'secondary_deps': models.SecondaryDepartment.objects.all(),
		'programming_lans': models.ProgrammingLanguage.objects.all(),
		'photo_path': './upload/users/photos',
		'reg_error': error
	}
	return context
	 

def register(request):
	if request.method == 'GET':
		return render(request, "users/register.html", __getContext(""))
	elif request.method == 'POST':
		user_name = request.POST['user_name']
		primary_dep = request.POST['primary_dep']
		secondary_dep = request.POST['secondary_dep']
		email = request.POST['email']
		programming_lans = request.POST.getlist('programming_lan')
		password = request.POST['password']
		confirm_pwd = request.POST['confirm_pwd']

		# If user exists
		if auth_models.User.objects.filter(username=user_name):
			return render(request, "users/register.html", __getContext("用户已存在"))

		# Password confirm error?
		if password != confirm_pwd:
			return render(request, "users/register.html", __getContext("密码确认错误"))

		# Create auth user
		user = auth_models.User.objects.create_user(user_name, email, password)
	   
		# Create user info
		models.UserInfo.objects.create(user_id=user, 
			primary_department=models.PrimaryDepartment.objects.get(
				primary_department_id=primary_dep), 
			secondary_department=models.SecondaryDepartment.objects.get(
				secondary_department_id=secondary_dep))

		# Login user
		auth_login(request, user) 

		return HttpResponseRedirect("/main/")
