from django.db import models

# Create your models here.

class PrimaryDepartment(models.Model):
	primary_department_id = models.AutoField(primary_key=True)
	department_name = models.CharField(max_length=128)

class SecondaryDepartment(models.Model):
	secondary_department_id = models.AutoField(primary_key=True)
	department_name = models.CharField(max_length=128)

class ProgrammingLanguage(models.Model):
	programming_language_id = models.AutoField(primary_key=True)
	language_name = models.CharField(max_length=64)

class User(models.Model):
	user_id = models.AutoField(primary_key=True)
	primary_department_id = models.ForeignKey(PrimaryDepartment, on_delete=models.CASCADE)
	secondary_department_id = models.ForeignKey(SecondaryDepartment, on_delete=models.CASCADE)
	user_name = models.CharField(max_length=64)
	password = models.CharField(max_length=64)
	email = models.CharField(max_length=128)
	photo = models.CharField(max_length=256) # photo file path
	programming_languages = models.CharField(max_length=256) # language list, Json format
