import sys
sys.path.append("..")

from django.db import models
from users.models import User
from activities.models import Activity

# Create your models here.

class Vote(models.Model):
	vote_id = models.AutoField(primary_key=True)
	activity_id = models.ForeignKey(Activity, on_delete=models.CASCADE)
	vote_desc = models.CharField(max_length=1024)
	vote_start_time = models.DateTimeField()
	vote_end_time = models.DateTimeField()

class VoteItem(models.Model):
	vote_item_id = models.AutoField(primary_key=True)
	vote_id = models.ForeignKey(Vote, on_delete=models.CASCADE)
	user_id = models.ForeignKey(User, on_delete=models.CASCADE)
	vote_users = models.CharField(max_length=4096)
