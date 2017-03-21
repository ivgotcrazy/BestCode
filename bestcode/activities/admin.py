from django.contrib import admin

from .models import Activity, ActivityPlan, ActivityState

# Register your models here.

admin.site.register(Activity)
admin.site.register(ActivityPlan)
admin.site.register(ActivityState)
