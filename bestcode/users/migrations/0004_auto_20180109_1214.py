# -*- coding: utf-8 -*-
# Generated by Django 1.10 on 2018-01-09 12:14
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('users', '0003_auto_20180109_0608'),
    ]

    operations = [
        migrations.AlterField(
            model_name='activityuser',
            name='photo',
            field=models.ImageField(upload_to='/media/users/photos'),
        ),
    ]
