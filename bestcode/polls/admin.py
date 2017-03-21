from django.contrib import admin

from .models import Vote, VoteItem

# Register your models here.

admin.site.register(Vote)
admin.site.register(VoteItem)
