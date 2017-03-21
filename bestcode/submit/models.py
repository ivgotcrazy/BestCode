import sys
sys.path.append("..")

from django.db import models

from users.models import ProgrammingLanguage, User
from activities.models import Activity


# Create your models here.

class Submit(models.Model):
	submit_id = models.AutoField(primary_key=True)
	user_id = models.ForeignKey(User, on_delete=models.CASCADE)
	activity_id = models.ForeignKey(Activity, on_delete=models.CASCADE)
	programming_language_id = models.ForeignKey(ProgrammingLanguage, on_delete=models.CASCADE)

class SubmitFileType(models.Model):
	submit_file_type_id = models.AutoField(primary_key=True)
	sumbmit_file_type_name = models.CharField(max_length=64)

class SubmitFile(models.Model):
	submit_file_id = models.AutoField(primary_key=True)
	submit_file_type_id = models.ForeignKey(SubmitFileType, on_delete=models.CASCADE)
	submit_id = models.ForeignKey(Submit, on_delete=models.CASCADE)
	file_path = models.CharField(max_length=256)	

# this should be in activities app, but for cycle dependence
class ReviewPlan(models.Model):
    review_plan_id = models.AutoField(primary_key=True)
    submit_id = models.ForeignKey(Submit, on_delete=models.CASCADE)
    plan_item_start = models.DateTimeField('Start time')
    plan_item_end = models.DateTimeField('End time')

# this should be in activities app, but for cycle dependence
class ActivityReviewer(models.Model):
    activity_reviewer_id = models.AutoField(primary_key=True)
    submit_id = models.ForeignKey(Submit, on_delete=models.CASCADE)
    user_id = models.ForeignKey(User, on_delete=models.CASCADE)
