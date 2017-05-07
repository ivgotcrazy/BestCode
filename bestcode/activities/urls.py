from django.conf.urls import url
from . import views

urlpatterns = [
    url(r'(?P<activity_id>[0-9]\d*)', views.activity, name='activity'),
]

