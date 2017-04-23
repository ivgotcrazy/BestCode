import sys
sys.path.append("..")

from django.db import models
from django.contrib.auth import models as auth_models

# Create your models here.

class ActivityState(models.Model):
	state_id = models.AutoField(primary_key=True)
	state_name = models.CharField(max_length=64)

	def __str__(self):
		return self.state_name

class Activity(models.Model):
	activity_id = models.AutoField(primary_key=True)
	activity_name = models.CharField(max_length=64, default="none")
	activity_desc = models.TextField(max_length=4096)
	activity_result = models.TextField(max_length=4096)
	state_id = models.ForeignKey(ActivityState, on_delete=models.CASCADE)

	def __str__(self):
		return self.activity_name 
	
class ActivityPlan(models.Model):
	activity_plan_id = models.AutoField(primary_key=True)
	activity = models.ForeignKey(Activity, on_delete=models.CASCADE)
	plan_item_name = models.CharField(max_length=64)
	plan_item_start = models.DateTimeField('Start time')
	plan_item_end = models.DateTimeField('End time')

	def __str__(self):
		return "activity plan of %s" % self.activity_id
