import sys
sys.path.append("..")

from django.db import models
from django.contrib.auth import models as auth_models
from activities.models import Activity

# Create your models here.

class Vote(models.Model):
    vote_id = models.AutoField(primary_key=True)
    activity = models.ForeignKey(Activity, on_delete=models.CASCADE)
    vote_desc = models.CharField(max_length=1024)
    vote_start_time = models.DateTimeField()
    vote_end_time = models.DateTimeField()

    def __str__(self):
        return self.vote_desc

class VoteItem(models.Model):
    vote_item_id = models.AutoField(primary_key=True)
    vote_item_name = models.CharField(max_length=64, default="none")
    vote = models.ForeignKey(Vote, on_delete=models.CASCADE)
    user = models.ForeignKey(auth_models.User, on_delete=models.CASCADE)
    vote_users = models.CharField(max_length=4096)

    def __str__(self):
        return self.vote_item_name
