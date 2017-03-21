import sys
sys.path.append("..")

from django.db import models
from users.models import User

# Create your models here.

class ActivityState(models.Model):
	state_id = models.AutoField(primary_key=True)
	state_name = models.CharField(max_length=64)

class Activity(models.Model):
	activity_id = models.AutoField(primary_key=True)
	activity_desc = models.TextField(max_length=4096)
	activity_result = models.TextField(max_length=4096)
	state_id = models.ForeignKey(ActivityState, on_delete=models.CASCADE)
	
class ActivityPlan(models.Model):
	activity_plan_id = models.AutoField(primary_key=True)
	activity_id = models.ForeignKey(Activity, on_delete=models.CASCADE)
	plan_item_name = models.CharField(max_length=64)
	plan_item_start = models.DateTimeField('Start time')
	plan_item_end = models.DateTimeField('End time')
